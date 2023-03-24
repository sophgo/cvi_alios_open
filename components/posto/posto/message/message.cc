/*
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 */

#include <cstring>
#include <memory>

#include "posto/message/message.h"
#include "posto/transport/ioblock/amp_ioblock.h"

namespace posto {

size_t MessageBase::ByteSizeLong() const {
  // header
  size_t size = sizeof(MessageHeader);
  // size of ioblock: uint64_t
  size += 8;
  // ioblock metas
  size += io_blocks_.size() * sizeof(transport::IoBlockMeta);
  // body size
  size += _ByteSizeLong();
  return size;
}

bool MessageBase::SerializeToArray(void* _data, size_t _size) const {
  char* data = (char*)_data;
  // header
  size_t size = sizeof(MessageHeader);
  std::memcpy(data, &header_, sizeof(MessageHeader));
  // ioblocks
  *(uint64_t*)(data + size) = io_blocks_.size();
  size += 8;
  for (auto blk : io_blocks_) {
    transport::IoBlockMeta meta;
    std::dynamic_pointer_cast<transport::AmpIoBlock>(blk)->SerializeTo(meta);
    std::memcpy(data + size, &meta, sizeof(transport::IoBlockMeta));
    size += sizeof(transport::IoBlockMeta);
  }
  // body
  return _SerializeToArray(data + size, _size - size);
}

bool MessageBase::ParseFromArray(const void* _data, size_t _size) {
  char* data = (char*)_data;
  // header
  size_t size = sizeof(MessageHeader);
  std::memcpy(&header_, data, sizeof(MessageHeader));
  // ioblocks
  size_t num_blks = *(uint64_t*)(data + size);
  size += 8;
  while (num_blks--) {
    auto blk = std::make_shared<transport::AmpIoBlock>();
    blk->DeserializeFrom(*(transport::IoBlockMeta*)(data + size));
    io_blocks_.emplace_back(blk);
    size += sizeof(transport::IoBlockMeta);
  }
  // body
  return _ParseFromArray(data + size, _size - size);
}

}  // namespace posto
