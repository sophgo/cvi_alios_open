/*
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 */

#include "posto/base/host.h"

#ifdef CONFIG_COMP_IPC
#define POSTO_AMP_ENABLE
#endif

#ifdef __YOC__
#ifdef POSTO_AMP_ENABLE
extern "C" int32_t drv_get_cpu_id(void);
#endif
#endif

namespace posto {
namespace base {
namespace host {

uint64_t get_id() {
#ifdef __YOC__
#ifdef POSTO_AMP_ENABLE
  static int32_t _cpu_id = drv_get_cpu_id();
  return (uint64_t)_cpu_id;
#else
  return (uint64_t)0;
#endif
#endif
  return 0xFFFFFFFFFFFFFFFF;
}

}  // namespace host 
}  // namespace base
}  // namespace posto
