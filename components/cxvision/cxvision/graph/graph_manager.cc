/*
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 */

#include <string.h>
#ifdef CXVISION_STATISTICS_ENABLE
#include <aos/cli.h>
#endif
#include <posto/base/host.h>
#include <posto/base/mutex.h>

#include "cxvision/graph/config.h"
#include "cxvision/graph/graph_agent.h"
#include "cxvision/graph/graph_manager.h"

namespace cx {
namespace {

struct StatisticsHelper {
  static void cmd_func(char *wbuf, int wbuf_len, int argc, char **argv);

  static void Init();
  static void Add(GraphManager* mgr);
  static void Remove(GraphManager* mgr);

  static posto::base::mutex mutex_;
  static std::vector<GraphManager*> graph_mgrs_;
  static bool inited_;
};

posto::base::mutex StatisticsHelper::mutex_;
std::vector<GraphManager*> StatisticsHelper::graph_mgrs_;
//bool StatisticsHelper::inited_{false};
#if 0
void StatisticsHelper::cmd_func(char *wbuf, int wbuf_len, int argc, char **argv) {
  if (argc == 1 || 0 == strcmp(argv[1], "statistics")) {
    mutex_.lock();
    for (auto mgr : graph_mgrs_) {
      mgr->PrintStatistics();
    }
    mutex_.unlock();
  } else {
    printf("usage: cxvision {statistics}\n");
  }
}
#endif
void StatisticsHelper::Init() {
/* TODO
  static const struct cli_command _cmd_info = {
      "cxvision",
      "cxvision statistics",
      &StatisticsHelper::cmd_func
  };

  if (!inited_) {
    mutex_.lock();
    if (!inited_) {
      inited_ = true;
      aos_cli_register_command(&_cmd_info);
    }
    mutex_.unlock();
  }
*/
}

void StatisticsHelper::Add(GraphManager* mgr) {
  mutex_.lock();
  graph_mgrs_.push_back(mgr);
  mutex_.unlock();
}

void StatisticsHelper::Remove(GraphManager* mgr) {
  mutex_.lock();
  for (auto it = graph_mgrs_.begin(); it != graph_mgrs_.end(); ++it) {
    if (*it == mgr) {
      graph_mgrs_.erase(it);
      break;
    }
  }
  mutex_.unlock();
}

}  // namespace

GraphManager::GraphManager(const std::string& jstr) : json_(jstr) {
  posto::Init();
  StatisticsHelper::Init();
  StatisticsHelper::Add(this);
}

GraphManager::~GraphManager() {
  StatisticsHelper::Remove(this);
}

bool GraphManager::Start() {
  std::string host_id = std::to_string(posto::base::host::get_id());
  // host id/pid/object address
  std::string manager_id = host_id + "/0/" + std::to_string((uintptr_t)this);
  std::vector<cx::config::Pipeline> pipelines;
  if (cx::config::LoadFromJson(json_, pipelines)) {
    for (const auto& pipeline : pipelines) {
      cx::config::Graph graph;
      if (cx::config::CreateGraph(pipeline, graph)) {
        for (const auto& v : graph.vertices) {
          if (v.node->device_id.empty() || v.node->device_id == host_id) {
            auto mgr = cx::plugin::Manager::Instance();
            auto plugin = mgr->Create(v.node->plugin);
            if (plugin) {
              plugin->_CreateIoPorts(manager_id, graph, v);
              plugin->Init(v.node->props);
              plugin->_Start();
              auto& plugins = graphs_[graph.name];
              plugins.push_back({v.node->name, plugin});
            }
          } else {
            auto it = agents_.find(v.node->device_id);
            if (it == agents_.end()) {
              auto agent_client = std::make_shared<internal::GraphAgentClient>(
                  manager_id, v.node->device_id);
              agent_client->Init();
              agent_client->Start(json_);
              agents_[v.node->device_id] = agent_client;
            }
          }
        }
      }
    }
  }
  return true;
}

bool GraphManager::Stop() {
  // TODO
  return true;
}

void GraphManager::PrintStatistics() {
  for (const auto& gpair : graphs_) {
    TAB_PRINT(0, "pipeline: %s\r\n", gpair.first.c_str());
    for (const auto& vtx_info : gpair.second) {
      TAB_PRINT(1, "node: %s\r\n", vtx_info.name.c_str());
      vtx_info.plugin->_PrintStatistics(2);
    }
  }
}

}  // namespace cx
