/*
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 */

#ifdef CONFIG_COMP_IPC
#define POSTO_AMP_ENABLE
#endif

#ifdef POSTO_AMP_ENABLE
#include <ipc.h>
#endif

#include "posto/transport/amp/amp_ipc.h"

#define AMP_IPC_SERVICE_ID (0xEE)

#define LOG_TAG "AmpIpc"

#ifdef POSTO_AMP_ENABLE
extern "C" int32_t drv_get_cpu_id(void);
#ifndef POSTO_AMP_NUM_CPUS
#define POSTO_AMP_NUM_CPUS 2
#endif
#endif

namespace posto {
namespace {

#ifdef POSTO_AMP_ENABLE
struct _amp_ipc_context {
  int32_t self_cpu_id = -1;
  ipc_t* ipc_set[POSTO_AMP_NUM_CPUS];
  IpcMessageHandler* sd_handler_ptr;
  IpcMessageHandler* shm_ref_handler;
  IpcMessageHandler* user_msg_handler;
} __attribute__((aligned(64)));

struct _amp_ipc_context g_amp_ipc;

void amp_ipc_msg_cb(ipc_t *ipc, message_t *msg, void *priv) {
  uint32_t cpu_id_from = (uint32_t)(uintptr_t)priv;
  switch (msg->command) {
  case AmpIpc::SD_PORT:
    (*g_amp_ipc.sd_handler_ptr)((uint8_t*)msg->req_data,
                                (size_t)msg->req_len, cpu_id_from);
    break;
  case AmpIpc::SHM_REF_PORT:
    (*g_amp_ipc.shm_ref_handler)((uint8_t*)msg->req_data,
                                 (size_t)msg->req_len, cpu_id_from);
    break;
  case AmpIpc::USER_MSG_PORT:
    (*g_amp_ipc.user_msg_handler)((uint8_t*)msg->req_data,
                                  (size_t)msg->req_len, cpu_id_from);
    break;
  default:
    break;
  }
}

inline bool amp_ipc_init() {
  int cpu_set[POSTO_AMP_NUM_CPUS] = { 0 };
  ipc_t **curr = &g_amp_ipc.ipc_set[0];
  g_amp_ipc.self_cpu_id = drv_get_cpu_id();

  cpu_set[g_amp_ipc.self_cpu_id] = -1;
  for (int i = 0; i < POSTO_AMP_NUM_CPUS; i++) {
    if (cpu_set[i]) {
      *curr++ = nullptr;
      continue;
    }

    *curr = ipc_get(i);
    if (*curr) {
      ipc_add_service(*curr, AMP_IPC_SERVICE_ID, amp_ipc_msg_cb, (void *)(uintptr_t)i);
    }
    ++curr;
  }
  return true;
}

inline int amp_ipc_sendto(uint8_t* data, size_t size,
                          uint32_t cpu_id, uint16_t port) {
  auto* hipc = g_amp_ipc.ipc_set[cpu_id];
  if (!hipc) {
    return 0;
  }

  message_t snd_msg;
  snd_msg.command    = port;
  snd_msg.flag       = MESSAGE_ASYNC;
  snd_msg.service_id = AMP_IPC_SERVICE_ID;
  snd_msg.req_data   = data;
  snd_msg.req_len    = size;
  snd_msg.resp_data  = NULL;
  snd_msg.resp_len   = 0;
  if (ipc_message_send(hipc, &snd_msg, AOS_WAIT_FOREVER)) {
    return -1;
  }
  return (int)size;
}

inline void amp_ipc_broadcast(uint8_t* data, size_t size, uint16_t port) {
  for (uint32_t cpu_id = 0; cpu_id < POSTO_AMP_NUM_CPUS; ++cpu_id) {
    amp_ipc_sendto(data, size, cpu_id, port);
  }
}
#endif

} // namespace

AmpIpc::AmpIpc() = default;

bool AmpIpc::Init() {
#ifdef POSTO_AMP_ENABLE
  // The caller must make sure handlers are set before calling Init()
  g_amp_ipc.sd_handler_ptr = &sd_handler_;
  g_amp_ipc.shm_ref_handler = &shm_ref_handler_;
  g_amp_ipc.user_msg_handler = &user_msg_handler_;
  return amp_ipc_init();
#else
  return true;
#endif
}

bool AmpIpc::Bind(uint16_t port, const IpcMessageHandler& msg_handler) {
#ifdef POSTO_AMP_ENABLE
  switch (port) {
  case SD_PORT:
    sd_handler_ = msg_handler;
    break;
  case SHM_REF_PORT:
    shm_ref_handler_ = msg_handler;
    break;
  case USER_MSG_PORT:
    user_msg_handler_ = msg_handler;
    break;
  default: {
      base::lock_guard<base::mutex> lock(mutex_);
      const auto& it = listeners_.find(port);
      if (it != listeners_.end())
        return false;

      listeners_[port] = msg_handler;
    }
    break;
  }
  return true;
#else
  (void) port;
  (void) msg_handler;
  return true;
#endif
}

void AmpIpc::Unbind(uint16_t port) {
  // TODO: Unbind
}

int AmpIpc::SendTo(uint8_t* data, size_t size, uint32_t cpu_id,
    uint16_t port) {
#ifdef POSTO_AMP_ENABLE
  return amp_ipc_sendto(data, size, cpu_id, port);
#else
  (void) data;
  (void) size;
  (void) cpu_id;
  (void) port;
  return -1;
#endif
}

void AmpIpc::Broadcast(uint8_t* data, size_t size, uint16_t port) {
#ifdef POSTO_AMP_ENABLE
  amp_ipc_broadcast(data, size, port);
#else
  (void) data;
  (void) size;
  (void) port;
#endif
}

}  // namespace posto
