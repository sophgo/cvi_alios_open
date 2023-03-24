/*
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 */

#include "posto/base/host.h"
#include "posto/base/mutex.h"
#include "posto/participant/participant.h"

namespace posto {

Participant::Participant(const std::string& name) : entity_counter_(0) {
  attributes().host_id = base::host::get_id();
  attributes().participant_name = name;
}

uint32_t Participant::NewEntityId() {
  static base::mutex _mtx;

  base::lock_guard<base::mutex> lock(_mtx);
  return ++entity_counter_;
}

}  // namespace posto
