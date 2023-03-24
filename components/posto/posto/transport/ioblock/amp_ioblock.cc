/*
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 */

#include <assert.h>
#include <stdio.h>
#include <aos/kernel.h>
#include <csi_core.h>

#include <cstdint>
#include <cstring>

#include "posto/base/host.h"
#include "posto/base/mutex.h"
#include "posto/transport/amp/amp_ipc.h"
#include "posto/transport/ioblock/amp_ioblock.h"

#define ALIGN_SIZE 64
#define AMP_ALIGN(value) (((value) + (ALIGN_SIZE - 1)) & ~(ALIGN_SIZE - 1))

#define LOG_ENABLE 0
#define LOG_TAG "AmpIoBlock"

static uint32_t g_self_id;
static posto::base::mutex g_mutex;
static std::map<uint64_t, int32_t> g_shm_refs; // ID:Ref-count pair

extern "C" {
__attribute__((weak)) unsigned long chip_shm_address_mapping(uint32_t self_id, uint32_t owner, unsigned long addr) {
  return addr;
}
}

namespace posto {
namespace transport {

enum ShmProc : uint8_t {
  SHM_PROC_REF,
  SHM_PROC_UNREF,
};

struct ShmCommand {
  uint64_t id;
  uint8_t proc;
  uint8_t _reserved[7];
};

static_assert(sizeof(ShmCommand) == 16, "ShmCommand size error");

AmpShm::AmpShm() : id_(0) {}

bool AmpShm::Build(uint64_t id, uint32_t owner, bool is_create) {
  id_ = id;
  owner_ = owner;

  if (is_create) {
    base::lock_guard<base::mutex> lock(g_mutex);
    auto iter = g_shm_refs.find(id_);
    if (iter != g_shm_refs.end()) {
      printf("[Warning] AmpShm::Build %d \r\n", __LINE__);
    }

    g_shm_refs[id_] = 1;
  }
  return true;
}

void AmpShm::Ref() {
  if (g_self_id != owner_) {
    ShmCommand cmd{ id_, SHM_PROC_REF };
    AmpIpc::Instance()->SendTo((uint8_t*)&cmd, sizeof(ShmCommand),
        owner_, AmpIpc::SHM_REF_PORT);
  } else {
    base::lock_guard<base::mutex> lock(g_mutex);
    auto iter = g_shm_refs.find(id_);
    if (iter == g_shm_refs.end()) {
      printf("[Warning] AmpShm::Ref %d \r\n", __LINE__);
    }
#if LOG_ENABLE
    printf("[%s][CPU%d]Ref, owner: %d, id: %p, ref_count: %d\r\n",
        LOG_TAG, (int)g_self_id, (int)owner_,
        (void *)(uintptr_t)id_, (int)g_shm_refs[id_]);
#endif

    ++g_shm_refs[id_];
  }
}

void AmpShm::Unref() {
  if (g_self_id != owner_) {
    ShmCommand cmd{ id_, SHM_PROC_UNREF };
    AmpIpc::Instance()->SendTo((uint8_t*)&cmd, sizeof(ShmCommand),
        owner_, AmpIpc::SHM_REF_PORT);
  } else {
    base::lock_guard<base::mutex> lock(g_mutex);
    auto iter = g_shm_refs.find(id_);
    if (iter == g_shm_refs.end()) {
      printf("[Warning] AmpShm::Unref %d \r\n", __LINE__);
      return;
    }
#if LOG_ENABLE
    printf("[%s][CPU%d]Unref, owner: %d, id: %p, ref_count: %d\r\n",
        LOG_TAG, (int)g_self_id, (int)owner_,
        (void *)(uintptr_t)id_, (int)g_shm_refs[id_]);
#endif

    int32_t _count = --g_shm_refs[id_];
    if (_count == 0) {
      auto it = g_shm_refs.find(id_);
      g_shm_refs.erase(it);
#if LOG_ENABLE
      printf("[%s][CPU%d]Free: %p\r\n", LOG_TAG, (int)g_self_id,
          (void *)(uintptr_t)id_);
#endif
      aos_free((void *)(uintptr_t)id_);
    }
  }
}

// static
bool AmpIoBlock::Init() {
  g_self_id = (uint32_t)base::host::get_id();
  return AmpIpc::Instance()->Bind(AmpIpc::SHM_REF_PORT,
      [] (uint8_t* data, size_t size, uint32_t cpu_id_from) {
    ShmCommand* cmd = (ShmCommand*)data;
#if LOG_ENABLE
    printf("[%s][CPU%d]From: %d, %s: %p\r\n", LOG_TAG,
        (int)g_self_id, (int)cpu_id_from,
        (cmd->proc == SHM_PROC_REF) ? "Ref" : "Unref",
        (void *)(uintptr_t)cmd->id);
#endif
    AmpShm _shm;
    _shm.Build(cmd->id, g_self_id, false);
    if (cmd->proc == SHM_PROC_REF) {
      _shm.Ref();
    } else if (cmd->proc == SHM_PROC_UNREF) {
      _shm.Unref();
    }
  });
}

AmpIoBlock::AmpIoBlock()
    : block_size_(0),
      block_addr_(nullptr),
      size_(0),
      data_(nullptr) {}

AmpIoBlock::AmpIoBlock(size_t size) : AmpIoBlock() {
  size_t sz_blk = AMP_ALIGN(size + ALIGN_SIZE);
  block_addr_ = (char*)aos_malloc(sz_blk);
#if LOG_ENABLE
  printf("[%s][CPU%d]Malloc: %p\r\n", LOG_TAG, (int)g_self_id, block_addr_);
#endif
  if (block_addr_) {
    block_size_ = sz_blk;
    size_ = size;
    data_ = (char*)AMP_ALIGN((uintptr_t)block_addr_);
    shm_.Build((uint64_t)block_addr_, g_self_id, true);
  }
}

AmpIoBlock::AmpIoBlock(const void* data, size_t size) : AmpIoBlock(size) {
  std::memcpy(data_, data, size);
}

AmpIoBlock::~AmpIoBlock() {
  if (shm_.id() != 0) {
    shm_.Unref();
  }
}

bool AmpIoBlock::SerializeTo(IoBlockMeta& meta) {
  meta.id = shm_.id();
  meta.owner = shm_.owner();
  meta.data_size = size_;
  meta.block_size = block_size_;
  // add ref before writing
  shm_.Ref();
  auto base = (char*)AMP_ALIGN((uintptr_t)block_addr_);
#if defined(__riscv) && (__riscv_xlen == 64)
  csi_dcache_clean_range((uint64_t*)base, AMP_ALIGN(size_));
#else
  csi_dcache_clean_range((uint32_t*)base, AMP_ALIGN(size_));
#endif
  return true;
}

bool AmpIoBlock::DeserializeFrom(const IoBlockMeta& meta) {
  block_size_ = meta.block_size;
  size_ = meta.data_size;

  block_addr_ = (char*)chip_shm_address_mapping(g_self_id, (uint32_t)meta.owner, (unsigned long)meta.id);

  auto base = (char*)AMP_ALIGN((uintptr_t)block_addr_);
#if defined(__riscv) && (__riscv_xlen == 64)
  csi_dcache_invalid_range((uint64_t*)base, AMP_ALIGN(size_));
#else
  csi_dcache_invalid_range((uint32_t*)base, AMP_ALIGN(size_));
#endif
  data_ = base;
  shm_.Build(meta.id, (uint32_t)meta.owner, false);
  return true;
}

void* AmpIoBlock::data() {
  return data_;
}

size_t AmpIoBlock::size() const {
  return size_;
}

size_t AmpIoBlock::block_size() const {
  return block_size_;
}

uint64_t AmpIoBlock::id() const {
  return shm_.id();
}

}  // namespace transport
}  // namespace posto
