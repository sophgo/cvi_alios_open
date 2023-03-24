/*
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 */

#include <cstdint>
#include <utility>

#include "cxvision/plugin/buffer.h"

#define _MY_ALIGN(x,a) _MY_ALIGN_MASK(x,(typeof(x))(a)-1)
#define _MY_ALIGN_MASK(x,mask) (((x)+(mask))&~(mask))
#define _ALIGN_4(x) _MY_ALIGN(x,4)
#define _ALIGN_8(x) _MY_ALIGN(x,8)

namespace cx {

size_t Buffer::_ByteSizeLong() const {
  // metas' size
  size_t size = 4;
  for (const auto& meta : metas_) {
    // key
    size += 4;
    size += _ALIGN_4(meta.first.size());

    // value
    size += 4;
#ifndef CXVISION_USE_PROTOBUF
    auto obj = std::static_pointer_cast<posto::Serializable>(meta.second.object);
    size = _ALIGN_8(size);
    size += _ALIGN_4(obj->ByteSizeLong());
#endif
  }
  return size;
}

bool Buffer::_SerializeToArray(void* data, size_t size) const {
  auto ptr = (char*)data;
  *(uint32_t*)ptr = metas_.size();
  ptr += 4;
  for (const auto& meta : metas_) {
    // key
    *(uint32_t*)ptr = meta.first.size();
    ptr += 4;
    std::memcpy(ptr, meta.first.data(), meta.first.size());
    ptr += _ALIGN_4(meta.first.size());

    // value
#ifndef CXVISION_USE_PROTOBUF
    auto obj = std::static_pointer_cast<posto::Serializable>(meta.second.object);
    auto obj_size = obj->ByteSizeLong();
    *(uint32_t*)ptr = obj_size;
    ptr += 4;
    ptr = (char*)_ALIGN_8((uintptr_t)ptr);
    obj->SerializeToArray(ptr, obj_size);
    ptr += _ALIGN_4(obj_size);
#endif
  }
  return true;
}

bool Buffer::_ParseFromArray(const void* data, size_t size) {
  auto ptr = (char*)data;
  auto meta_size = *(uint32_t*)ptr;
  ptr += 4;
  for (unsigned i = 0; i < meta_size; ++i) {
    // key
    auto key_size = *(uint32_t*)ptr;
    ptr += 4;
    std::string key(ptr, key_size);
    ptr += _ALIGN_4(key_size);

    // value
    auto value_size = *(uint32_t*)ptr;
    ptr += 4;
    ptr = (char*)_ALIGN_8((uintptr_t)ptr);
    std::string value(ptr, value_size);
    ptr += _ALIGN_4(value_size);

    metas_[key].view = std::move(value);
  }
  return true;
}

}  // namespace cx
