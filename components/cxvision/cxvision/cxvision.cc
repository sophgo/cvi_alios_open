/*
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 */

#include <posto/base/mutex.h>
#include <posto/posto.h>

#include "cxvision/cxvision.h"
#include "cxvision/graph/graph_agent.h"

static bool g_inited;
static posto::base::mutex g_mutex;

namespace cx {

bool Init() {
  posto::base::lock_guard<posto::base::mutex> lock(g_mutex);
  if (g_inited)
    return true;

  bool ret = posto::Init();
  ret |= cx::internal::GraphAgent::Instance()->Init();
  g_inited = ret;
  return ret;
}

}  // namespace cx
