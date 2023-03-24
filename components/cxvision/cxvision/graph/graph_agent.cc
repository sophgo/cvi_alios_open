/*
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 */

#include <posto/base/host.h>

#include "cxvision/graph/config.h"
#include "cxvision/graph/graph_agent.h"
#include "cxvision/plugin/plugin.h"

#define REQUEST_TOPIC_PREFIX "/cx/agent/req/"
#define RESPONSE_TOPIC_PREFIX "/cx/agent/res/"

namespace cx {
namespace internal {

// class GraphAgent
GraphAgent::GraphAgent() {}

bool GraphAgent::Init() {
  std::string req_topic = std::string(REQUEST_TOPIC_PREFIX) +
      std::to_string(posto::base::host::get_id());
  participant_ = posto::Domain::CreateParticipant("any");
  req_channel_ = participant_->CreateReader<RequestT>(req_topic,
      [this](const RequestTPtr& req) {
    std::string res_topic = std::string(RESPONSE_TOPIC_PREFIX) +
        req->body().manager_id() + ":" +
        std::to_string(posto::base::host::get_id());
    std::shared_ptr<posto::Writer<ResponseT>> res_channel;
    auto it = res_channels_.find(res_topic);
    if (it == res_channels_.end()) {
      res_channel = participant_->CreateWriter<ResponseT>(res_topic);
      res_channels_[res_topic] = res_channel;
    } else {
      res_channel = it->second;
    }

    int ret;
    auto res = std::make_shared<ResponseT>();
    switch (req->body().proc_id()) {
    case AGENT_PROC_START:
      ret = this->Start(req->body().manager_id(), req->body().config());
      break;
    case AGENT_PROC_STOP:
      ret = this->Stop(req->body().manager_id());
      break;
    case AGENT_PROC_GET_STATISTICS: {
        std::string out_str;
        ret = this->GetStatistics(req->body().manager_id(), out_str);
        res->body().set_statistics(out_str);
      }
      break;
    default:
      ret = -1;
      break;
    }

    res->body().set_ctx_id(req->body().ctx_id());
    res->body().set_error_code(ret);
    res_channel->Write(res);
  });
  return true;
}

int GraphAgent::Start(const std::string& manager_id, const std::string& conf) {
  std::string host_id = std::to_string(posto::base::host::get_id());
  std::vector<cx::config::Pipeline> pipelines;
  if (cx::config::LoadFromJson(conf, pipelines)) {
    for (const auto& pipeline : pipelines) {
      cx::config::Graph graph;
      if (cx::config::CreateGraph(pipeline, graph)) {
        for (const auto& v : graph.vertices) {
          if (v.node->device_id == host_id) {
            auto mgr = cx::plugin::Manager::Instance();
            auto plugin = mgr->Create(v.node->plugin);
            if (plugin) {
              plugin->_CreateIoPorts(manager_id, graph, v);
              plugin->Init(v.node->props);
              plugin->_Start();
              auto& graphs = managers_[manager_id];
              auto& plugins = graphs[graph.name];
              plugins.push_back({v.node->name, plugin});
            }
          }
        }
      }
    }
  }
  return true;
}

int GraphAgent::Stop(const std::string& manager_id) {
  // TODO
  return 0;
}

int GraphAgent::GetStatistics(const std::string& manager_id,
                              std::string& out_str) {
  // TODO
  return 0;
}

// class GraphAgentClient
GraphAgentClient::GraphAgentClient(const std::string& manager_id,
                                   const std::string& dest_device_id)
    : next_ctx_id_(0),manager_id_(manager_id), dest_device_id_(dest_device_id)
    {}

bool GraphAgentClient::Init() {
  std::string res_topic = std::string(RESPONSE_TOPIC_PREFIX) + manager_id_ +
      ":" + dest_device_id_;
  participant_ = posto::Domain::CreateParticipant("any");
  res_channel_ = participant_->CreateReader<ResponseT>(res_topic,
      [this](const ResponseTPtr& res) {
    auto ctx_id = res->body().ctx_id();
    PendingRequest* ctx = nullptr;
    mutex_.lock();
    auto it = pending_requests_.find(ctx_id);
    if (it != pending_requests_.end()) {
      ctx = it->second;
    }
    mutex_.unlock();
    if (ctx != nullptr) {
      ctx->response = res;
      aos_sem_signal(&ctx->notifier);
    }
  });
  std::string req_topic = std::string(REQUEST_TOPIC_PREFIX) + dest_device_id_;
  req_channel_ = participant_->CreateWriter<RequestT>(req_topic);
  return true;
}

bool GraphAgentClient::DeInit() {
  // TODO
  return true;
}

int GraphAgentClient::Start(const std::string& conf) {
  PendingRequest ctx;
  mutex_.lock();
  auto ctx_id = ++next_ctx_id_;
  pending_requests_[ctx_id] = &ctx;
  mutex_.unlock();

  auto req = std::make_shared<RequestT>();
  req->body().set_ctx_id(ctx_id);
  req->body().set_proc_id(AGENT_PROC_START);
  req->body().set_manager_id(manager_id_);
  req->body().set_config(conf);

  aos_sem_new(&ctx.notifier, 0);
  bool ret = req_channel_->Write(req);
  while (!ret) {
    aos_msleep(100);
    ret = req_channel_->Write(req);
    // TODO: break policy
  }
  aos_sem_wait(&ctx.notifier, AOS_WAIT_FOREVER);

  // Cleanup
  aos_sem_free(&ctx.notifier);
  mutex_.lock();
  auto it = pending_requests_.find(ctx_id);
  if (it != pending_requests_.end()) {
    pending_requests_.erase(it);
  }
  mutex_.unlock();

  return ctx.response->body().error_code();
}

int GraphAgentClient::Stop() {
  return 0;
}

int GraphAgentClient::GetStatistics(std::string& out_str) {
  return 0;
}

}  // namespace internal
}  // namespace cx
