/*
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 */

#include <cstdint>

#include "posto/base/mutex.h"
#include "posto/domain/domain.h"

namespace posto {
namespace {

uint32_t NewParticipantId() {
  static base::mutex _mtx;
  static uint32_t _next_id = 0;

  base::lock_guard<base::mutex> lock(_mtx);
  return ++_next_id;
}

}  // namespace

// static
std::shared_ptr<Participant>
Domain::CreateParticipant(const std::string& name) {
  auto id = NewParticipantId();
  auto parti = new Participant(name);
  entity::guid::GeneratePrefix(parti->guid(), id);
  parti->attributes().participant_id = id;
  return std::shared_ptr<Participant>(parti);
}

}  // namespace posto
