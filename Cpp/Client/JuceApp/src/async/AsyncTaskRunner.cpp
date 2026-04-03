#include "tempolink/juce/async/AsyncTaskRunner.h"

#include <utility>

namespace tempolink::juceapp::async {

AsyncTaskRunner::AsyncTaskRunner(std::shared_ptr<std::atomic_bool> alive_flag,
                                 int num_threads)
    : alive_flag_(std::move(alive_flag)) {
  workers_.reserve(static_cast<std::size_t>(num_threads));
  for (int i = 0; i < num_threads; ++i) {
    workers_.emplace_back([this] { WorkerLoop(); });
  }
}

AsyncTaskRunner::~AsyncTaskRunner() { Shutdown(); }

void AsyncTaskRunner::RunAsync(Task task) {
  if (shutting_down_.load()) return;
  {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    tasks_.push(std::move(task));
  }
  queue_cv_.notify_one();
}

void AsyncTaskRunner::Shutdown() {
  shutting_down_ = true;
  queue_cv_.notify_all();
  for (auto& worker : workers_) {
    if (worker.joinable()) worker.join();
  }
  workers_.clear();
}

void AsyncTaskRunner::WorkerLoop() {
  while (true) {
    Task task;
    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      queue_cv_.wait(lock, [this] {
        return shutting_down_.load() || !tasks_.empty();
      });
      if (shutting_down_.load() && tasks_.empty()) return;
      task = std::move(tasks_.front());
      tasks_.pop();
    }
    if (task) task();
  }
}

}  // namespace tempolink::juceapp::async
