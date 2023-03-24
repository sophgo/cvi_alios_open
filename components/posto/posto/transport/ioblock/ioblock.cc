/*
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 */

#include "posto/transport/ioblock/ioblock.h"
#include "posto/transport/ioblock/amp_ioblock.h"

namespace posto {
namespace transport {

// static
bool IoBlock::Init() {
  return AmpIoBlock::Init();
}

// static
IoBlockPtr IoBlock::New(size_t size) {
  return std::make_shared<AmpIoBlock>(size);
}

//static
IoBlockPtr IoBlock::Fill(const void *data, size_t size) {
  return std::make_shared<AmpIoBlock>(data, size);
}

IoBlock::~IoBlock() {}

}  // namespace transport
}  // namespace posto
