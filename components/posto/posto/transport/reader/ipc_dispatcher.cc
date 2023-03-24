/*
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 */

#include <cstddef>
#include <cstring>

#include "posto/message/message.h"
#include "posto/transport/reader/ipc_dispatcher.h"

namespace posto {
namespace transport {

IpcDispatcher::IpcDispatcher() = default;
IpcDispatcher::~IpcDispatcher() = default;

bool IpcDispatcher::Init() {
  return AmpIpc::Instance()->Bind(AmpIpc::USER_MSG_PORT,
      [this] (uint8_t* data, size_t size, uint32_t cpu_id_from) {
    this->OnData(data, size);
  });
}

void IpcDispatcher::AddListener(const entity::Attributes& attr,
    const IpcListener& listener) {
  base::lock_guard<base::mutex> lock(mutex_);
  // replace anyway
  listeners_[attr.guid] = listener;
}

void IpcDispatcher::RemoveListener(const entity::Attributes& attr) {
  base::lock_guard<base::mutex> lock(mutex_);
  auto it = listeners_.find(attr.guid);
  if (it != listeners_.end()) {
    listeners_.erase(it);
  }
}

void IpcDispatcher::OnData(uint8_t* data, size_t size) {
  entity::Guid guid;
  std::memcpy(guid.value,
      data + offsetof(MessageHeader, dst_id), entity::Guid::kSize);

  IpcListener listener;
  {
    base::lock_guard<base::mutex> lock(mutex_);
    {
      auto it = listeners_.find(guid);
      if (it != listeners_.end()) {
        listener = it->second;
      }
    }
  }

  if (listener) {
    listener(data, size);
  } else {
    // TODO: Release OS-level resources in message, e.g. Shared memory
  }
}

}  // namespace transport
}  // namespace posto
