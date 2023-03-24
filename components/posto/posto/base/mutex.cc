/*
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 */

#include "posto/base/mutex.h"

#ifdef __YOC__
#include <aos/kernel.h>
#define AOS_MUTEX_PTR(storage) reinterpret_cast<aos_mutex_t*>(storage)
#else
#include <mutex>
#endif

namespace posto {
namespace base {

mutex::mutex() {
#ifdef __YOC__
  static_assert(sizeof(aos_mutex_t) <= sizeof(mutex::data_),
      "size of aos_mutex_t overflow.");
  aos_mutex_t *m = AOS_MUTEX_PTR(data_);
  aos_mutex_new(m);
#else
  static_assert(sizeof(std::mutex) <= sizeof(mutex::data_), "");
  std::mutex *mtx = new (data_) std::mutex();
  (void)mtx;
#endif
}

mutex::~mutex() {
#ifdef __YOC__
  aos_mutex_free(AOS_MUTEX_PTR(data_));
#else
  std::mutex *mtx = reinterpret_cast<std::mutex*>(data_);
  mtx->~mutex();
#endif
}

void mutex::lock() {
#ifdef __YOC__
  aos_mutex_lock(AOS_MUTEX_PTR(data_), AOS_WAIT_FOREVER);
#else
  std::mutex *mtx = reinterpret_cast<std::mutex*>(data_);
  mtx->lock();
#endif
}

void mutex::unlock() {
#ifdef __YOC__
  aos_mutex_unlock(AOS_MUTEX_PTR(data_));
#else
  std::mutex *mtx = reinterpret_cast<std::mutex*>(data_);
  mtx->unlock();
#endif
}

} // namespace base
} // namespace posto
