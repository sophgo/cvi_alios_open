/*
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 */

#include "posto/base/mutex.h"
#include "posto/discovery/endpoint_manager.h"
#include "posto/posto.h"
#include "posto/transport/amp/amp_ipc.h"
#include "posto/transport/reader/ipc_dispatcher.h"

namespace posto {
namespace {

bool g_inited;
base::mutex g_mutex;

}  // namespace

bool Init() {
  base::lock_guard<base::mutex> lock(g_mutex);
  if (g_inited)
    return true;

  bool ret = transport::IoBlock::Init();
  ret |= transport::IpcDispatcher::Instance()->Init();
  ret |= discovery::EndpointManager::Instance()->Init();
  ret |= AmpIpc::Instance()->Init();
  g_inited = ret;
  return ret;
}

}  // namespace posto
