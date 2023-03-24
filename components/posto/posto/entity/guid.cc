/*
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 */

#include "posto/base/host.h"
#include "posto/entity/guid.h"

#define UINT8_CAST(value) (unsigned char)((value) & 0xFF)

namespace posto {
namespace entity {
namespace guid {

void GeneratePrefix(Guid& guid, uint32_t participant_id) {
  uint16_t host_id = (uint16_t)base::host::get_id();
  guid.value[0] = 'T';
  guid.value[1] = 'H';
  guid.value[4] = UINT8_CAST(host_id >> 8);
  guid.value[5] = UINT8_CAST(host_id);
  guid.value[8] = UINT8_CAST(participant_id >> 24);
  guid.value[9] = UINT8_CAST(participant_id >> 16);
  guid.value[10] = UINT8_CAST(participant_id >> 8);
  guid.value[11] = UINT8_CAST(participant_id);
}

void SetEntityId(Guid& guid, uint32_t entity_id) {
  guid.value[12] = UINT8_CAST(entity_id >> 24);
  guid.value[13] = UINT8_CAST(entity_id >> 16);
  guid.value[14] = UINT8_CAST(entity_id >> 8);
  guid.value[15] = UINT8_CAST(entity_id);
}

}  // namespace guid
}  // namespace entity
}  // namespace posto
