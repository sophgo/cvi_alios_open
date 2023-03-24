/*
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <algorithm>
#include <cstring>
#include <utility>

#include "posto/base/host.h"
#include "posto/discovery/discovery.h"
#include "posto/discovery/endpoint_manager.h"
#include "posto/transport/amp/amp_ipc.h"

#define LOG_TAG "EndpointManager"

namespace posto {
namespace discovery {

namespace {

size_t make_change_message(ActionType action,
                           EntityType type,
                           const entity::Attributes& attr,
                           uint8_t* payload) {
  size_t size = 0;
  // action
  *(uint32_t*)payload = action;
  size += sizeof(uint32_t);
  // entity type
  *(uint32_t*)(payload + size) = type;
  size += sizeof(uint32_t);
  // guid
  std::memcpy(payload + size, attr.guid.value, entity::Guid::kSize);
  size += entity::Guid::kSize;
  // host ID
  *(uint64_t*)(payload + size) = attr.host_id;
  size += sizeof(uint64_t);
  // participant ID
  *(uint64_t*)(payload + size) = attr.participant_id;
  size += sizeof(uint64_t);
  // topic
  size_t topic_size = attr.topic.size();
  *(uint32_t*)(payload + size) = topic_size;
  size += sizeof(uint32_t);
  std::memcpy(payload + size, attr.topic.data(), topic_size);
  size += topic_size;
  return size;
}

bool parse_change_message(ActionType& action,
                          EntityType& type,
                          entity::Attributes& attr,
                          uint8_t* payload,
                          size_t _size) {
  size_t size = 0;
  // action
  action = (ActionType)*(uint32_t*)payload;
  size += sizeof(uint32_t);
  // entity type
  type = (EntityType)*(uint32_t*)(payload + size);
  size += sizeof(uint32_t);
  // guid
  std::memcpy(attr.guid.value, payload + size, entity::Guid::kSize);
  size += entity::Guid::kSize;
  // host ID
  attr.host_id = *(uint64_t*)(payload + size);
  size += sizeof(uint64_t);
  // participant ID
  attr.participant_id = *(uint64_t*)(payload + size);
  size += sizeof(uint64_t);
  // topic
  size_t topic_size = *(uint32_t*)(payload + size);
  size += sizeof(uint32_t);
  attr.topic = std::string((char *)payload + size, topic_size);
  size += topic_size;
  return (size == _size);
}

void publish_change(ActionType action,
                    EntityType type,
                    const entity::Attributes& attr) {
  alignas(4) uint8_t buf[256];
  size_t size = make_change_message(action, type, attr, buf);
  if (size > 0) {
    printf("[%s][CPU%d]Send action: %s, type: %s, topic: [%s]\r\n",
           LOG_TAG,
           (int)base::host::get_id(),
           discovery::Action2Name(action),
           entity::Entity2Name(type),
           attr.topic.data());
    AmpIpc::Instance()->Broadcast(buf, size, AmpIpc::SD_PORT);
  }
}

} // namespace

EndpointManager::EndpointManager() = default;
EndpointManager::~EndpointManager() = default;

bool EndpointManager::Init() {
  // Add handler first
  return AmpIpc::Instance()->Bind(AmpIpc::SD_PORT, [this] (uint8_t* data,
      size_t size, uint32_t cpu_id_from) {
    (void)cpu_id_from;
    ActionType action;
    EntityType type;
    entity::Attributes attr;
    if (parse_change_message(action, type, attr, data, size)) {
      printf("[%s][CPU%d]Recv action: %s, type: %s, topic: [%s]\r\n",
             LOG_TAG,
             (int)base::host::get_id(),
             discovery::Action2Name(action),
             entity::Entity2Name(type),
             attr.topic.data());
      if (action == ACTION_JOIN)
        this->Join(attr, type);
      else
        this->Leave(attr, type);
    }
  });
}

bool EndpointManager::Join(const entity::Attributes& attr, EntityType type) {
  auto _search = [] (const std::list<EntityAttrPtr>& attr_list,
                     const entity::Attributes& attr) -> bool {
    auto it = std::find_if(attr_list.begin(), attr_list.end(),
        [&attr] (const EntityAttrPtr& attr_ptr) {
      return (attr_ptr->host_id == attr.host_id &&
              attr_ptr->participant_id == attr.participant_id &&
              attr_ptr->topic == attr.topic);
    });
    return (it != attr_list.end());
  };
  bool is_remote = (attr.host_id != base::host::get_id());

  base::lock_guard<base::mutex> lock(mutex_);
  if (ENTITY_WRITER == type) {
    if (_search(writers_, attr)) {
      return false;
    }

    if (is_remote) {
      for (const auto& reader : readers_) {
        // local && topic match
        if (reader->host_id == base::host::get_id() &&
            reader->topic == attr.topic) {
          // re-publish if remote writer is added later than local reader
          publish_change(ACTION_JOIN, ENTITY_READER, *reader);
        }
      }
    }
    writers_.emplace_back(std::make_shared<entity::Attributes>(attr));
  } else if (ENTITY_READER == type) {
    if (_search(readers_, attr)) {
      return false;
    }

    readers_.emplace_back(std::make_shared<entity::Attributes>(attr));
  }
  if (!is_remote) {
    publish_change(ACTION_JOIN, type, attr);
  }
  return true;
}

bool EndpointManager::Leave(const entity::Attributes& attr, EntityType type) {
  // TODO: Leave
  return true;
}

void EndpointManager::GetReadersByTopic(const std::string& topic,
                                        EntityAttrVec& out_readers) {
  base::lock_guard<base::mutex> lock(mutex_);
  for (auto& reader : readers_) {
    if (reader->topic == topic) {
      out_readers.push_back(reader);
    }
  }
}

}  // namespace discovery
}  // namespace posto
