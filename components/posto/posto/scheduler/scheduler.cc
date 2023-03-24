/*
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 */

#include <stddef.h>
#include <string.h>

#include <atomic>
#include <list>
#include <utility>
#include <vector>

#ifdef __YOC__
#include <aos/kernel.h>
#include <drv/tick.h>
#endif

#include "posto/base/mutex.h"
#include "posto/scheduler/scheduler.h"

#define DEFAULT_POOL_SIZE (2)
#ifdef CONFIG_POSTO_DEFAULT_TASK_SIZE
#define DEFAULT_STACK_SIZE CONFIG_POSTO_DEFAULT_TASK_SIZE
#else
#define DEFAULT_STACK_SIZE (8 * 1024)
#endif
#define DEFAULT_THREAD_PRIO (28)
#define POOL_NAME_FORMAT "pool_%p"
#define USER_NAME_FORMAT "user_%p"

namespace posto {
namespace scheduler {

// get CORE timer counter value (ticks)
inline uint64_t get_core_timer_counter_value() {
  return csi_tick_get_us();
}

#if defined(CONFIG_DEBUG) && CONFIG_DEBUG
struct TaskStatistics : public profile::Statistics {
  struct Cost {
    unsigned min;
    unsigned max;
    uint64_t total;
  };

  struct QueueWaterMark {
    unsigned max;
    unsigned current;
  };

  unsigned processed = 0;
  Cost cost{0xFFFFFFFF, 0, 0};
  QueueWaterMark queue_water_mark{0, 0};

  void Print(int level) override {
    TAB_PRINT(level, "processed: %u\r\n", processed);
    TAB_PRINT(level, "cost(tick): (max: %u, min: %u, avg: %u)\r\n",
        cost.max, cost.min, processed ? (unsigned)(cost.total / processed) : 0);
    TAB_PRINT(level, "queue watermark: (max: %u, current: %u)\r\n",
        queue_water_mark.max, queue_water_mark.current);
  };
};
#endif

// Task impl
class TaskImpl final {
public:
  inline bool Enqueue(const std::shared_ptr<void>& message);
  inline std::shared_ptr<void> Dequeue();
  inline void Run(const std::shared_ptr<void>& message);

  inline profile::Statistics* GetStatistics();

  base::mutex mutex_;
  std::list<std::shared_ptr<void>> messages_;
  TaskFunc callback_;
  aos_sem_t* notifier_;

#if defined(CONFIG_DEBUG) && CONFIG_DEBUG
  TaskStatistics statistics_;
#endif
};

bool TaskImpl::Enqueue(const std::shared_ptr<void>& message) {
  // TODO: Full policy
  mutex_.lock();
  messages_.push_back(message);
#if defined(CONFIG_DEBUG) && CONFIG_DEBUG
  unsigned size = messages_.size();
  if (size > statistics_.queue_water_mark.max) {
    statistics_.queue_water_mark.max = size;
  }
  statistics_.queue_water_mark.current = size;
#endif
  mutex_.unlock();
  aos_sem_signal(notifier_);
  return true;
}

std::shared_ptr<void> TaskImpl::Dequeue() {
  mutex_.lock();
  if (messages_.size() > 0) {
    auto msg = messages_.front();
    messages_.pop_front();
#if defined(CONFIG_DEBUG) && CONFIG_DEBUG
    statistics_.queue_water_mark.current = messages_.size();
#endif
    mutex_.unlock();
    return msg;
  }
  mutex_.unlock();
  return nullptr;
}

void TaskImpl::Run(const std::shared_ptr<void>& message) {
#if defined(CONFIG_DEBUG) && CONFIG_DEBUG
  auto t0 = get_core_timer_counter_value();
#endif
  callback_(message);
#if defined(CONFIG_DEBUG) && CONFIG_DEBUG
  // TODO: Consider counter value overflow
  unsigned cost = (unsigned)(get_core_timer_counter_value() - t0);
  if (cost < statistics_.cost.min)
    statistics_.cost.min = cost;
  if (cost > statistics_.cost.max)
    statistics_.cost.max = cost;
  statistics_.cost.total += cost;
  ++statistics_.processed;
#endif
}

profile::Statistics* TaskImpl::GetStatistics() {
#if defined(CONFIG_DEBUG) && CONFIG_DEBUG
  return &statistics_;
#else
  return nullptr;
#endif
}

// class Task
Task::Task() : impl(new TaskImpl()) {}
bool Task::Enqueue(const std::shared_ptr<void>& message) {
  return impl->Enqueue(message);
}

profile::Statistics* Task::GetStatistics() {
  return impl->GetStatistics();
}

// Executor impl
class ExecutorImpl final {
public:
  static void thread_entry(void *arg);

  bool Init(const Executor::Attributes* attr, const char* name_fmt);
  void DeInit();
  inline void AddTask(const std::shared_ptr<Task>& task);
  inline bool RemoveTask(Task* task);

  char name_[32];
  aos_sem_t semaphore_;
  aos_task_t thread_;
  base::mutex mutex_;
  std::vector<std::shared_ptr<Task>> tasks_;
};

// static
void ExecutorImpl::thread_entry(void *arg) {
  auto self = static_cast<ExecutorImpl*>(arg);
  unsigned int counter = 0;

  while (true) {
    if (0 == aos_sem_wait(&self->semaphore_, AOS_WAIT_FOREVER)) {
      std::shared_ptr<Task> tsk;
      std::shared_ptr<void> msg;

      self->mutex_.lock();
      size_t size = self->tasks_.size();
      for (size_t i = 0; i < size; ++i) {
        const auto& task = self->tasks_.at(counter % size);
        ++counter;
        msg = task->impl->Dequeue();
        if (msg != nullptr) {
          tsk = task;
          break;
        }
      }
      self->mutex_.unlock();

      if (msg != nullptr) {
        tsk->impl->Run(msg);
      }
    }
  }

  aos_task_exit(0);
}

bool ExecutorImpl::Init(const Executor::Attributes* attr, const char* name_fmt) {
  int priority = attr ? attr->priority : DEFAULT_THREAD_PRIO;
  int stack_size = attr ? (int)attr->stack_size : DEFAULT_STACK_SIZE;

  snprintf(name_, sizeof(name_), name_fmt, this);
  aos_sem_new(&semaphore_, 0);
  aos_task_new_ext(&thread_, name_, &ExecutorImpl::thread_entry,
      this, stack_size, priority);
  return true;
}

void ExecutorImpl::DeInit() {
  // TODO: Join thread
  aos_sem_free(&semaphore_);
}

void ExecutorImpl::AddTask(const std::shared_ptr<Task>& task) {
  mutex_.lock();
  tasks_.push_back(task);
  mutex_.unlock();
}

bool ExecutorImpl::RemoveTask(Task* task) {
  bool ret = false;
  mutex_.lock();
  auto it = tasks_.begin();
  for (; it != tasks_.end(); ++it) {
    if ((*it).get() == task) {
      tasks_.erase(it);
      ret = true;
      break;
    }
  }
  mutex_.unlock();
  return ret;
}

// Executor
Executor::Executor() : impl(new ExecutorImpl()) {}

// Scheduler impl
class SchedulerImpl final {
public:
  SchedulerImpl();
  ~SchedulerImpl();

  Executor* CreateExecutor(const Executor::Attributes* attr);

  Task* CreateTask(TaskFunc&& func,
                   Executor* executor,
                   const Task::Attributes* attr);
  bool DestroyTask(Task* task);

private:
  std::atomic<bool> stop_;
  base::mutex mutex_;
  unsigned int balance_counter_;
  Executor fwk_executors_[DEFAULT_POOL_SIZE];
  std::list<std::shared_ptr<Executor>> usr_executors_;
};

SchedulerImpl::SchedulerImpl() : stop_(false), balance_counter_(0) {
  for (int i = 0; i < DEFAULT_POOL_SIZE; ++i) {
    fwk_executors_[i].impl->Init(nullptr, POOL_NAME_FORMAT);
  }
}

SchedulerImpl::~SchedulerImpl() {
  stop_ = true;
}

Executor* SchedulerImpl::CreateExecutor(const Executor::Attributes* attr) {
  auto executor = std::make_shared<Executor>();
  if (executor->impl->Init(attr, USER_NAME_FORMAT)) {
    mutex_.lock();
    usr_executors_.push_back(executor);
    mutex_.unlock();
    return executor.get();
  }
  return nullptr;
}

Task* SchedulerImpl::CreateTask(TaskFunc&& func,
                                Executor* executor,
                                const Task::Attributes* attr) {
  auto task = std::make_shared<Task>();
  ExecutorImpl* eimpl;

  mutex_.lock();
  if (executor) {
    eimpl = executor->impl.get();
  } else {
    auto selector = balance_counter_ % DEFAULT_POOL_SIZE;
    ++balance_counter_;
    eimpl = fwk_executors_[selector].impl.get();
  }
  task->impl->callback_ = std::move(func);
  task->impl->notifier_ = &eimpl->semaphore_;
  eimpl->AddTask(task);
  mutex_.unlock();
  return task.get();
}

bool SchedulerImpl::DestroyTask(Task* task) {
  mutex_.lock();
  for (int i = 0; i < DEFAULT_POOL_SIZE; ++i) {
    if (fwk_executors_[i].impl->RemoveTask(task))
      return true;
  }

  for (const auto& executor : usr_executors_) {
    if (executor->impl->RemoveTask(task))
      return true;
  }
  mutex_.unlock();
  return false;
}

// class Scheduler
Scheduler::Scheduler() : impl_(new SchedulerImpl()) {}
Scheduler::~Scheduler() {}

Executor* Scheduler::CreateExecutor(const Executor::Attributes* attr) {
  return impl_->CreateExecutor(attr);
}

Task* Scheduler::CreateTask(TaskFunc&& func,
                            Executor* executor,
                            const Task::Attributes* attr) {
  return impl_->CreateTask(std::move(func), executor, attr);
}

bool Scheduler::DestroyTask(Task* task) {
  return impl_->DestroyTask(task);
}

}  // namespace scheduler
}  // namespace posto
