#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <cstdlib>
//#include <sys/mman.h>

#include "bmruntime.h"
#include "cvi_device_mem.h"
#include "cvitpu_debug.h"

bmctx_t  CviDeviceMem::root_ctx_array[CTX_MAX_CNT] = {0};
uint16_t CviDeviceMem::root_submit_array[SUBMIT_QUEUE_MAX] = {0};
std::mutex CviDeviceMem::root_daemon_lock;
tee_firewall_info CviDeviceMem::root_tee_firewall_info[TEE_FIREWALL_MAX] = {0};

#define ROOTDAEMON_LOCK() root_daemon_lock.lock();
#define ROOTDAEMON_UNLOCK() root_daemon_lock.unlock();

#define ION_MEM_ALIGN 0x1000

CviDeviceMem::~CviDeviceMem() {}

#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))
#define ALIGN(x,a)              __ALIGN_MASK(x,(__typeof__(x))(a)-1)
#ifdef ION_CACHE_OPEN

bmerr_t CviDeviceMem::mem_flush_ext(bmdev_t dev, int dma_fd, uint64_t paddr, size_t size)
{
  int ret;
  struct bm_cache_op_arg flush_arg;
  flush_arg.paddr = paddr + GLOBAL_MEM_START_ADDR;
  flush_arg.size = size;
  flush_arg.dma_fd = 0;

  uint64_t addr_new = ALIGN(flush_arg.paddr, 0x40);
  flush_arg.size = ALIGN(flush_arg.size, 0x40);

  if (addr_new != flush_arg.paddr) {
    //TPU_LOG_WARNING("fix flush add_p=0x%lx, len=0x%llx, add_p_ori=0x%llx\n", addr_new - 0x40, flush_arg.size, flush_arg.paddr);
    flush_arg.size += 0x40;
    flush_arg.paddr = addr_new - 0x40;
  } else {
    //TPU_LOG_WARNING("ok flush add_p=0x%llx, len=0x%llx\n", flush_arg.paddr, flush_arg.size);
  }

  ret = cvi_tpu_ioctl(dev->dev_fd, CVITPU_DMABUF_FLUSH, reinterpret_cast<unsigned long>(&flush_arg));
  if (ret != 0) {
    TPU_LOG_WARNING("memory flush failed, ret=%x\n", ret);
    return BM_ERR_FAILURE;
  }
  return BM_SUCCESS;
}

bmerr_t CviDeviceMem::mem_invld_ext(bmdev_t dev, int dma_fd, uint64_t paddr, size_t size)
{
  int ret;
  struct bm_cache_op_arg invalidate_arg;
  invalidate_arg.paddr = paddr + GLOBAL_MEM_START_ADDR;
  invalidate_arg.size = size;
  invalidate_arg.dma_fd = 0;

  uint64_t addr_new = ALIGN(invalidate_arg.paddr, 0x40);
  invalidate_arg.size = ALIGN(invalidate_arg.size, 0x40);

  if (addr_new != invalidate_arg.paddr) {
    //TPU_LOG_WARNING("fix invalid add_p=0x%lx, len=0x%llx, add_p_ori=0x%llx\n", addr_new - 0x40, invalidate_arg.size, invalidate_arg.paddr);
    invalidate_arg.size += 0x40;
    invalidate_arg.paddr = addr_new - 0x40;
  } else {
    //TPU_LOG_WARNING("ok invalid add_p=0x%llx, len=0x%llx\n", invalidate_arg.paddr, invalidate_arg.size);
  }

  ret = cvi_tpu_ioctl(dev->dev_fd, CVITPU_DMABUF_INVLD, reinterpret_cast<unsigned long>(&invalidate_arg));
  if (ret != 0) {
    TPU_LOG_WARNING("memory invalidate failed, ret=%x\n", ret);
    return BM_ERR_FAILURE;
  }
  return BM_SUCCESS;
}

bmerr_t CviDeviceMem::mem_flush_addr(bmdev_t dev, uint64_t paddr, size_t size)
{
  int ret;
  struct bm_cache_op_arg flush_arg;
  flush_arg.paddr = paddr;
  flush_arg.size = size;
  flush_arg.dma_fd = 0;

  uint64_t addr_new = ALIGN(flush_arg.paddr, 0x40);
  flush_arg.size = ALIGN(flush_arg.size, 0x40);

  if (addr_new != flush_arg.paddr) {
    //TPU_LOG_WARNING("fix flush add_p=0x%lx, len=0x%llx, add_p_ori=0x%llx\n", addr_new - 0x40, flush_arg.size, flush_arg.paddr);
    flush_arg.size += 0x40;
    flush_arg.paddr = addr_new - 0x40;
  } else {
    //TPU_LOG_WARNING("ok flush add_p=0x%llx, len=0x%llx\n", flush_arg.paddr, flush_arg.size);
  }

  ret = cvi_tpu_ioctl(dev->dev_fd, CVITPU_DMABUF_FLUSH, reinterpret_cast<unsigned long>(&flush_arg));
  if (ret != 0) {
    TPU_LOG_WARNING("memory flush failed, ret=%x\n", ret);
    return BM_ERR_FAILURE;
  }
  return BM_SUCCESS;
}

bmerr_t CviDeviceMem::mem_invld_addr(bmdev_t dev, uint64_t paddr, size_t size)
{
  int ret;
  struct bm_cache_op_arg invalidate_arg;
  invalidate_arg.paddr = paddr;
  invalidate_arg.size = size;
  invalidate_arg.dma_fd = 0;

  uint64_t addr_new = ALIGN(invalidate_arg.paddr, 0x40);
  invalidate_arg.size = ALIGN(invalidate_arg.size, 0x40);

  if (addr_new != invalidate_arg.paddr) {
    //TPU_LOG_WARNING("fix invalid add_p=0x%lx, len=0x%llx, add_p_ori=0x%llx\n", addr_new - 0x40, invalidate_arg.size, invalidate_arg.paddr);
    invalidate_arg.size += 0x40;
    invalidate_arg.paddr = addr_new - 0x40;
  } else {
    //TPU_LOG_WARNING("ok invalid add_p=0x%llx, len=0x%llx\n", invalidate_arg.paddr, invalidate_arg.size);
  }

  ret = cvi_tpu_ioctl(dev->dev_fd, CVITPU_DMABUF_INVLD, reinterpret_cast<unsigned long>(&invalidate_arg));
  if (ret != 0) {
    TPU_LOG_WARNING("memory invalidate failed, ret=%x\n", ret);
    return BM_ERR_FAILURE;
  }
  return BM_SUCCESS;
}

#else

bmerr_t CviDeviceMem::mem_flush_ext(bmdev_t dev, int dma_fd, uint64_t paddr, size_t size)
{
  return BM_SUCCESS;
}

bmerr_t CviDeviceMem::mem_invld_ext(bmdev_t dev, int dma_fd, uint64_t paddr, size_t size)
{
  return BM_SUCCESS;
}

#endif

bmerr_t CviDeviceMem::submit_dmabuf(bmdev_t dev, uint64_t p_addr, uint32_t seq_no)
{
  struct bm_submit_dma_arg submit_dma_arg;
  submit_dma_arg.paddr = p_addr;
  submit_dma_arg.seq_no = seq_no;
  int ret = cvi_tpu_ioctl(dev->dev_fd, CVITPU_SUBMIT_DMABUF, reinterpret_cast<unsigned long>(&submit_dma_arg));
  TPU_ASSERT(ret == 0, "run dmabuf failed");
  return ret ? BM_ERR_FAILURE : BM_SUCCESS;
}

bmerr_t CviDeviceMem::wait_dmabuf(bmdev_t dev, uint32_t seq_no)
{
  struct bm_wait_dma_arg wait_dma_arg;
  wait_dma_arg.seq_no = seq_no;
  int ret = 0, loop_cnt = 0;

  do {
    if (loop_cnt > 10)
      TPU_LOG_WARNING("bm_device_wait_dmabuf() triggered loop=%d\n", loop_cnt);

    ret = cvi_tpu_ioctl(dev->dev_fd, CVITPU_WAIT_DMABUF, reinterpret_cast<unsigned long>(&wait_dma_arg));
    loop_cnt ++;
  } while (ret);

  TPU_ASSERT(wait_dma_arg.ret == 0, NULL);
  return BM_SUCCESS;
}

bmerr_t CviDeviceMem::mem_alloc(bmdev_t dev, size_t size, uint64_t *paddr,
                      uint8_t **vaddr, uint64_t *ori_addr)
{
  if (size == 0) {
    TPU_LOG_WARNING("can't alloc size 0\n");
    return BM_ERR_INVALID_ARGUMENT;
  }

  // addr align to 0x1000
  size_t align_size = size + ION_MEM_ALIGN;

  /* alloc buffer */
  void *p = malloc(align_size);
  if (!p) {
    TPU_LOG_WARNING("memory alloc failed\n");
    return BM_ERR_FAILURE;
  }

  memset(p, 0x00, align_size);

  *ori_addr = (uint64_t)p;
  *ori_addr -= GLOBAL_MEM_START_ADDR;
  *paddr = ALIGN((uint64_t)p, ION_MEM_ALIGN);
  *vaddr = (uint8_t *)*paddr;

  return BM_SUCCESS;
}

bmerr_t CviDeviceMem::mem_free(uint64_t ori_addr, size_t size)
{
  // align to 0x1000
  free((void *)ori_addr);
  return BM_SUCCESS;
}

bmerr_t CviDeviceMem::load_tee(bmctx_t ctx, uint64_t cmdbuf_addr_ree, uint32_t cmdbuf_len_ree,
                      uint64_t weight_addr_ree, uint32_t weight_len_ree,
                      uint64_t neuron_addr_ree)
{
  struct cvi_load_tee_arg iocrl_arg;

  //recover physical address for TEE 1 vs 1 memory mapping
  iocrl_arg.cmdbuf_addr_ree = cmdbuf_addr_ree + GLOBAL_MEM_START_ADDR;
  iocrl_arg.cmdbuf_len_ree = cmdbuf_len_ree;
  iocrl_arg.weight_addr_ree = weight_addr_ree + GLOBAL_MEM_START_ADDR;
  iocrl_arg.weight_len_ree = weight_len_ree;
  iocrl_arg.neuron_addr_ree = neuron_addr_ree + GLOBAL_MEM_START_ADDR;

  int ret = cvi_tpu_ioctl(ctx->dev->dev_fd, CVITPU_LOAD_TEE, reinterpret_cast<unsigned long>(&iocrl_arg));
  TPU_ASSERT(ret == 0, "cvi_device_load_tee() failed");
  return BM_SUCCESS;
}

bmerr_t CviDeviceMem::run_tee(bmctx_t ctx, uint32_t seq_no, uint64_t dmabuf_addr,
                      uint64_t array_base2, uint64_t array_base3,
                      uint64_t array_base4, uint64_t array_base5,
                      uint64_t array_base6, uint64_t array_base7)
{
  struct cvi_submit_tee_arg iocrl_arg;

  iocrl_arg.dmabuf_tee_addr = dmabuf_addr;
  iocrl_arg.gaddr_base2 = array_base2;
  iocrl_arg.gaddr_base3 = array_base3;
  iocrl_arg.gaddr_base4 = array_base4;
  iocrl_arg.gaddr_base5 = array_base5;
  iocrl_arg.gaddr_base6 = array_base6;
  iocrl_arg.gaddr_base7 = array_base7;
  iocrl_arg.seq_no = seq_no;

  int ret = cvi_tpu_ioctl(ctx->dev->dev_fd, CVITPU_SUBMIT_TEE, reinterpret_cast<unsigned long>(&iocrl_arg));
  TPU_ASSERT(ret == 0, "cvi_device_run_tee() failed");

  return BM_SUCCESS;
}


void CviDeviceMem::bmmem_dump_mem_array(void) {
  int total = 0;
  cvi_mem_pair *root_pair = NULL;

  for (int j = 0; j < CTX_MAX_CNT; j ++) {

    if (root_ctx_array[j]) {
      root_pair = root_ctx_array[j]->root_mem_array;

      for (int i = 0; i < MEMARRAY_MAX_CNT; i ++) {
        if (root_pair[i].p_addr) {
          TPU_LOG_DEBUG("%" PRIx64 ", index=%x\n", root_pair[i].p_addr, i);
          total ++;
        }
      }
    }
  }

  TPU_LOG_DEBUG("bmmem_dump_mem_array() cnt=%x\n", total);
}

bmmem_device_t CviDeviceMem::mem_alloc_raw(bmctx_t ctx, size_t size) {
  int ret = 0, i = 0;
  char array_got = 0;

  bm_memory_t *device_mem = new bm_memory_t();
  device_mem->flags.u.is_prealloc = 0;
  device_mem->flags.u.type = BMMEM_TYPE_DEVICE;
  device_mem->size = size;
  device_mem->user_ref_cnt = 0;
  ret = mem_alloc(ctx->dev, size, &device_mem->p_addr, &device_mem->v_addr,
                            &device_mem->ori_addr);

  if (ret != BM_SUCCESS) {
    delete device_mem;
    TPU_ASSERT(0, "alloc ion failed");
    return NULL;
  }

  ROOTDAEMON_LOCK();
  //only support alloc, not support for prealloc
  for (i = 0; i < MEMARRAY_MAX_CNT; i ++) {
    if (ctx->root_mem_array[i].p_addr == 0) {
      ctx->root_mem_array[i].p_addr = device_mem->p_addr;
      ctx->root_mem_array[i].mem = (void *)device_mem;
      array_got = 1;
      break;
    }
  }
  ROOTDAEMON_UNLOCK();

  if (!array_got)
    TPU_LOG_WARNING("bmmem_device_alloc_raw() alloc over %d\n", MEMARRAY_MAX_CNT);

  BMEMEM_DUMP();
  return (bmmem_device_t)device_mem;
}

bmmem_device_t CviDeviceMem::mem_prealloc_raw(bmctx_t ctx, bmmem_device_t mem, uint64_t offset,
                                         size_t size) {
  (void)ctx;
  TPU_ASSERT(mem != nullptr, nullptr);
  TPU_ASSERT(mem->size >= size + offset, nullptr);
  bm_memory_t *device_mem = new bm_memory_t();
  device_mem->flags.u.is_prealloc = 1;
  device_mem->flags.u.type = BMMEM_TYPE_DEVICE;
  device_mem->p_addr = ((bm_memory_t *)mem)->p_addr + offset;
  device_mem->v_addr = ((bm_memory_t *)mem)->v_addr + offset;
  device_mem->dma_fd = ((bm_memory_t *)mem)->dma_fd;
  device_mem->offset = offset;
  device_mem->size = size;
  device_mem->user_ref_cnt = 0;

  BMEMEM_DUMP();
  return (bmmem_device_t)device_mem;
}


void CviDeviceMem::mem_free_ex(uint64_t p_addr) {
  bm_memory_t *device_mem = NULL;
  cvi_mem_pair *root_pair = NULL;

  ROOTDAEMON_LOCK();
  for (int j = 0; j < CTX_MAX_CNT; j ++) {

    if (root_ctx_array[j]) {
      root_pair = root_ctx_array[j]->root_mem_array;

      for (int i = 0; i < MEMARRAY_MAX_CNT; i ++) {
        if (root_pair[i].p_addr == p_addr) {
          device_mem = (bm_memory_t *)(root_pair[i].mem);
          mem_free(device_mem->ori_addr, device_mem->size);

          root_pair[i].p_addr = 0;
          root_pair[i].mem = NULL;
          break;
        }
      }
    }
  }
  ROOTDAEMON_UNLOCK();

  BMEMEM_DUMP();
  delete device_mem;
}

size_t CviDeviceMem::mem_size(bmmem_device_t mem) {
  bm_memory_t *device_mem = (bm_memory_t *)mem;
  if (device_mem == NULL)
    return 0;
  TPU_ASSERT(device_mem->flags.u.type == BMMEM_TYPE_DEVICE, NULL);
  return device_mem->size;
}

uint64_t CviDeviceMem::mem_p_addr(bmmem_device_t mem) {
  bm_memory_t *device_mem = (bm_memory_t *)mem;
  if (device_mem == NULL)
    return 0;
  TPU_ASSERT(device_mem->flags.u.type == BMMEM_TYPE_DEVICE, NULL);
  // tl_gdma will add an GLOBAL_MEM_START_ADDR offset to ga
  return device_mem->p_addr;
}

uint8_t *CviDeviceMem::mem_v_addr(bmmem_device_t mem) {
  bm_memory_t *device_mem = (bm_memory_t *)mem;
  TPU_ASSERT(device_mem->flags.u.type == BMMEM_TYPE_DEVICE, NULL);
  return device_mem->v_addr;
}

int32_t CviDeviceMem::mem_inc_ref(bmmem_device_t mem) {
  bm_memory_t *device_mem = (bm_memory_t *)mem;
  TPU_ASSERT(device_mem->flags.u.type == BMMEM_TYPE_DEVICE, NULL);
  return (++device_mem->user_ref_cnt);
}

int32_t CviDeviceMem::mem_dec_ref(bmmem_device_t mem) {
  bm_memory_t *device_mem = (bm_memory_t *)mem;
  TPU_ASSERT(device_mem->flags.u.type == BMMEM_TYPE_DEVICE, NULL);
  return (--device_mem->user_ref_cnt);
}

bmerr_t CviDeviceMem::mem_memcpy_s2d(bmctx_t ctx, bmmem_device_t dst, uint8_t *src) {
  bm_memory_t *device_mem = (bm_memory_t *)dst;
  memcpy(device_mem->v_addr, src, device_mem->size);
  TPU_ASSERT((int)mem_flush_ext(ctx->dev, device_mem->dma_fd,
        device_mem->p_addr, device_mem->size) == BM_SUCCESS, NULL);
  return BM_SUCCESS;
}

bmerr_t CviDeviceMem::mem_memcpy_s2d_ex(bmctx_t ctx, bmmem_device_t dst, uint8_t *src, uint64_t offset,
                         size_t size) {
  bm_memory_t *device_mem = (bm_memory_t *)dst;
  memcpy(device_mem->v_addr + offset, src, size);
  TPU_ASSERT(mem_flush_ext(ctx->dev,  device_mem->dma_fd,
        device_mem->p_addr + offset, size) == BM_SUCCESS, NULL);
  return BM_SUCCESS;
}

bmerr_t CviDeviceMem::mem_memcpy_d2s(bmctx_t ctx, uint8_t *dst, bmmem_device_t src) {
  bm_memory_t *device_mem = (bm_memory_t *)src;
  TPU_ASSERT(mem_invld_ext(ctx->dev, device_mem->dma_fd,
        device_mem->p_addr, device_mem->size) == BM_SUCCESS, NULL);
  memcpy(dst, device_mem->v_addr, device_mem->size);
  return BM_SUCCESS;
}

bmerr_t CviDeviceMem::mem_memcpy_d2s_ex(bmctx_t ctx, uint8_t *dst, bmmem_device_t src, uint64_t offset,
                         size_t size) {
  bm_memory_t *device_mem = (bm_memory_t *)src;
  TPU_ASSERT(mem_invld_ext(ctx->dev, device_mem->dma_fd,
        device_mem->p_addr + offset, size) == BM_SUCCESS, NULL);
  memcpy(dst, (device_mem->v_addr + offset), size);
  return BM_SUCCESS;
}

bmerr_t CviDeviceMem::mem_device_flush(bmctx_t ctx, bmmem_device_t mem) {
  return mem_flush_ext(ctx->dev, mem->dma_fd, mem->p_addr, mem->size);
}

bmerr_t CviDeviceMem::mem_device_flush_len(bmctx_t ctx, bmmem_device_t mem, size_t len) {
  return mem_flush_ext(ctx->dev, mem->dma_fd, mem->p_addr, len);
}

bmerr_t CviDeviceMem::mem_device_invld(bmctx_t ctx, bmmem_device_t mem) {
  return mem_invld_ext(ctx->dev, mem->dma_fd, mem->p_addr, mem->size);
}

bmerr_t CviDeviceMem::mem_device_invld_len(bmctx_t ctx, bmmem_device_t mem, size_t len) {
  return mem_invld_ext(ctx->dev, mem->dma_fd, mem->p_addr, len);
}

bmerr_t CviDeviceMem::context_create(bmctx_t *ctx) {
  char array_got = 0;
  bm_context_t *pctx = new bm_context_t;

  // memset context
  memset(pctx, 0, sizeof(bm_context_t));
  pctx->dev = NULL;
  pctx->seq_no = 0;
  *ctx = pctx;

  ROOTDAEMON_LOCK();
  //assign into root
  for (int i = 0; i < CTX_MAX_CNT; i ++) {
    if (!root_ctx_array[i]) {
      root_ctx_array[i] = pctx;
      array_got = 1;
      break;
    }
  }
  ROOTDAEMON_UNLOCK();

  if (!array_got)
    TPU_LOG_WARNING("bm_context_create() over %d\n", CTX_MAX_CNT);

  return BM_SUCCESS;
}

void CviDeviceMem::context_destroy(bmctx_t ctx) {
  TPU_ASSERT(ctx != nullptr,nullptr);

  ROOTDAEMON_LOCK();
  //remove from root
  for (int i = 0; i < CTX_MAX_CNT; i ++) {
    if (root_ctx_array[i] == ctx) {
      root_ctx_array[i] = NULL;
      break;
    }
  }
  ROOTDAEMON_UNLOCK();

  delete ctx;
}

bmerr_t CviDeviceMem::bind_device(bmctx_t ctx, bmdev_t dev) {
  TPU_ASSERT(ctx != nullptr, nullptr);
  ctx->dev = dev;
  return BM_SUCCESS;
}

void CviDeviceMem::unbind_device(bmctx_t ctx) {
  TPU_ASSERT(ctx != nullptr, nullptr);
  ctx->dev = NULL;
}

bmdev_t CviDeviceMem::get_device(bmctx_t ctx) {
  TPU_ASSERT(ctx->dev != nullptr, NULL);
  return ctx->dev;
}


void CviDeviceMem::device_exit(bmctx_t ctx) {
  bmdev_t dev = ctx->dev;
  unbind_device(ctx);
  context_destroy(ctx);
  device_close(dev);
}

bmerr_t CviDeviceMem::device_init(int index, bmctx_t *ctx) {
  bmerr_t ret;
  TPU_ASSERT(index == 0, NULL);

  bmdev_t dev;
  ret = device_open(index, &dev);
  TPU_ASSERT(ret == BM_SUCCESS, NULL);

  ret = context_create(ctx);
  TPU_ASSERT(ret == BM_SUCCESS, NULL);

  ret = bind_device(*ctx, dev);
  TPU_ASSERT(ret == BM_SUCCESS, NULL);
  return BM_SUCCESS;
}


bmerr_t CviDeviceMem::send_cmdbuf(bmctx_t ctx, uint8_t *cmdbuf, size_t sz, uint16_t *seq_no) {
  int ret;
  bmmem_device_t cmdbuf_mem;

  ret = load_cmdbuf(ctx, cmdbuf, sz, 0, 0, false, &cmdbuf_mem);
  if (ret != BM_SUCCESS) {
    TPU_ASSERT(0, "load cmdbuf error");
    return BM_ERR_FAILURE;
  }

  ret = run_cmdbuf(ctx, cmdbuf_mem, seq_no);
  TPU_ASSERT(ret == BM_SUCCESS, NULL);

  ret = wait_cmdbuf_done(ctx, *seq_no);
  mem_free_raw(ctx, cmdbuf_mem);
  TPU_ASSERT(ret == BM_SUCCESS, NULL);

  return BM_SUCCESS;
}

bmerr_t CviDeviceMem::wait_cmdbuf_done(bmctx_t ctx, uint16_t seq_no) {
  return wait_dmabuf(ctx->dev, seq_no);
}

bmerr_t CviDeviceMem::wait_cmdbuf_all(bmctx_t ctx) {
  int i, ret;

  for (i = 0; i < SUBMIT_QUEUE_MAX; i ++) {
    if (root_submit_array[i]) {
      ret = wait_dmabuf(ctx->dev, root_submit_array[i]);
      TPU_ASSERT(ret == BM_SUCCESS, NULL);
      root_submit_array[i] = 0;
    }
  }
  return 0;
}

bmerr_t CviDeviceMem::run_cmdbuf_pio(bmctx_t ctx, uint8_t *cmdbuf, size_t sz) {
  (void)ctx;
  (void)cmdbuf;
  (void)sz;
  TPU_ASSERT(0, NULL); // not support
  return BM_SUCCESS;
}

void CviDeviceMem::set_base_reg(bmctx_t ctx, uint32_t inx, uint64_t addr) {
  // currently we set base_select0 = neuron
  // base_select1 = weight
  // WARNING: this api is not thread-safe, only used in verification.
  if (inx == 0) {
    ctx->array_base0 = addr;
  } else if (inx == 1) {
    ctx->array_base1 = addr;
  } else {
    TPU_ASSERT(0, NULL);      //not supported
  }
  return;
}

uint64_t CviDeviceMem::read_base_reg(bmctx_t ctx, u32 inx) {
  // currently we set base_select0 = neuron
  // base_select1 = weight
  if (inx == 0) {
    return ctx->array_base0;
  } else if (inx == 1) {
    return ctx->array_base1;
  } else {
    TPU_ASSERT(0, NULL);      //not supported
  }
  return 0;
}

bmerr_t CviDeviceMem::run_cmdbuf_tee(bmctx_t ctx, uint16_t *seq_no, uint64_t dmabuf_addr, cvi_array_base *array_base)
{
  // seq_no need be protected
  BMDEV_LOCK(ctx->dev);

  dmabuf_addr += GLOBAL_MEM_START_ADDR;
  bmerr_t ret = run_tee(ctx, ctx->seq_no, dmabuf_addr,
                  array_base->gaddr_base2, array_base->gaddr_base3,
                  array_base->gaddr_base4, array_base->gaddr_base5,
                  array_base->gaddr_base6, array_base->gaddr_base7);

  *seq_no = ctx->seq_no++;
  BMDEV_UNLOCK(ctx->dev);
  return ret;
}

bmerr_t CviDeviceMem::run_cmdbuf(bmctx_t ctx, bmmem_device_t cmdbuf_mem, uint16_t *seq_no) {
  // seq_no need be protected
  BMDEV_LOCK(ctx->dev);

  bm_memory_t *device_mem = (bm_memory_t *)cmdbuf_mem;
  bmerr_t ret = submit_dmabuf(ctx->dev, device_mem->p_addr, ctx->seq_no);

  if (ret != BM_SUCCESS) {
    // ret = bmmem_device_crc32_check(ctx, cmdbuf_mem);
    // TPU_LOG_WARNING("run dambuf failed, crc32 check:" <;
  }

  *seq_no = ctx->seq_no++;
  BMDEV_UNLOCK(ctx->dev);

  return ret ? BM_ERR_FAILURE : BM_SUCCESS;
}

bmerr_t CviDeviceMem::run_cmdbuf_ex(bmctx_t ctx, bmmem_device_t cmdbuf_mem, uint16_t *seq_no,
                       uint64_t input_base_addr, uint64_t output_base_addr) {
  // seq_no need be protected
  BMDEV_LOCK(ctx->dev);

  bm_memory_t *device_mem = (bm_memory_t *)cmdbuf_mem;

  //assign input/output base selection
  dma_hdr_t *header = (dma_hdr_t *)(device_mem->v_addr);
  if (header->dmabuf_magic_m != tpu_dmabuf_header_m) {
    TPU_ASSERT(0, NULL);
    return BM_ERR_FAILURE;
  }

  //chip define arraybase_0 activation/neuron
  //chip define arraybase_1 weight
  //chip define arraybase_2 input
  //chip define arraybase_3 output
  header->arraybase_2_L = (uint32_t)input_base_addr;
  header->arraybase_2_H = 0;
  header->arraybase_3_L = (uint32_t)output_base_addr;
  header->arraybase_3_H = 0;

  //need not flush, bacause cmdbuf submit to kernel has remap mechanism
  //bm_device_mem_flush_ext(ctx->dev, device_mem->dma_fd, device_mem->p_addr, device_mem->size);

  //submit
  bmerr_t ret = submit_dmabuf(ctx->dev, device_mem->p_addr, ctx->seq_no);
  *seq_no = ctx->seq_no++;
  BMDEV_UNLOCK(ctx->dev);

  return ret ? BM_ERR_FAILURE : BM_SUCCESS;
}

bmerr_t CviDeviceMem::run_cmdbuf_ex2(bmctx_t ctx, bmmem_device_t cmdbuf_mem, uint16_t *seq_no,
                       cvi_array_base *p_array_base) {
  // seq_no need be protected
  BMDEV_LOCK(ctx->dev);

  bm_memory_t *device_mem = (bm_memory_t *)cmdbuf_mem;

  //assign input/output base selection
  dma_hdr_t *header = (dma_hdr_t *)(device_mem->v_addr);
  if (header->dmabuf_magic_m != tpu_dmabuf_header_m) {
    TPU_ASSERT(0, NULL);
    return BM_ERR_FAILURE;
  }

  //chip define arraybase_0 activation/neuron
  //chip define arraybase_1 weight
  //chip define arraybase_3 input
  //chip define arraybase_4 output
  header->arraybase_0_L = (uint32_t)p_array_base->gaddr_base0;
  header->arraybase_1_L = (uint32_t)p_array_base->gaddr_base1;
  header->arraybase_2_L = (uint32_t)p_array_base->gaddr_base2;
  header->arraybase_3_L = (uint32_t)p_array_base->gaddr_base3;
  header->arraybase_4_L = (uint32_t)p_array_base->gaddr_base4;
  header->arraybase_5_L = (uint32_t)p_array_base->gaddr_base5;
  header->arraybase_6_L = (uint32_t)p_array_base->gaddr_base6;
  header->arraybase_7_L = (uint32_t)p_array_base->gaddr_base7;

  //submit
  bmerr_t ret = submit_dmabuf(ctx->dev, device_mem->p_addr, ctx->seq_no);
  *seq_no = ctx->seq_no++;
  BMDEV_UNLOCK(ctx->dev);

  return ret ? BM_ERR_FAILURE : BM_SUCCESS;
}

bmerr_t CviDeviceMem::run_async(bmctx_t ctx, bmmem_device_t cmdbuf_mem)
{
  uint16_t seq_no_current = 0;
  int i;

  // seq_no need be protected
  BMDEV_LOCK(ctx->dev);
  bm_memory_t *device_mem = (bm_memory_t *)cmdbuf_mem;

  //submit
  bmerr_t ret = submit_dmabuf(ctx->dev, device_mem->p_addr, ctx->seq_no);
  seq_no_current = ctx->seq_no++;
  BMDEV_UNLOCK(ctx->dev);

  ROOTDAEMON_LOCK();
  for (i = 0; i < SUBMIT_QUEUE_MAX; i ++) {
    if (!root_submit_array[i]) {
      root_submit_array[i] = seq_no_current;
      break;
    }
  }
  ROOTDAEMON_UNLOCK();

  return ret ? BM_ERR_FAILURE : BM_SUCCESS;
}
