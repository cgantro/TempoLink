#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <condition_variable>

#include <juce_events/juce_events.h>

namespace tempolink::juceapp::async {

/// Safe async task runner. Replaces std::thread(...).detach() with a
/// managed thread pool that respects object lifetimes.
///
/// Key safety features:
/// - Tasks run on a fixed-size thread pool (no unbounded thread creation)
/// - Callbacks are dispatched via juce::MessageManager::callAsync
/// - alive_flag is checked before dispatching to prevent use-after-destroy
/// - Destructor drains the queue and joins all threads
class AsyncTaskRunner {
 public:
  using Task = std::function<void()>;

  explicit AsyncTaskRunner(std::shared_ptr<std::atomic_bool> alive_flag,
                           int num_threads = 2);
  ~AsyncTaskRunner();

  AsyncTaskRunner(const AsyncTaskRunner&) = delete;
  AsyncTaskRunner& operator=(const AsyncTaskRunner&) = delete;

  /// Run a task on a worker thread. The task runs in the background.
  void RunAsync(Task task);

  /// Run a task on a worker thread and dispatch the callback on the
  /// message thread. The callback is only invoked if alive_flag is true.
  template <typename WorkFn, typename CallbackFn>
  void RunAsyncThen(WorkFn work, CallbackFn callback) {
    auto alive = alive_flag_;
    RunAsync([work = std::move(work), callback = std::move(callback),
              alive]() mutable {
      auto result = work();
      juce::MessageManager::callAsync(
          [alive, callback = std::move(callback),
           result = std::move(result)]() mutable {
            if (!alive->load()) return;
            callback(std::move(result));
          });
    });
  }

  /// Shutdown the runner — stops accepting new tasks and joins threads.
  void Shutdown();

 private:
  void WorkerLoop();

  std::shared_ptr<std::atomic_bool> alive_flag_;
  std::vector<std::thread> workers_;
  std::queue<Task> tasks_;
  std::mutex queue_mutex_;
  std::condition_variable queue_cv_;
  std::atomic_bool shutting_down_{false};
};

}  // namespace tempolink::juceapp::async
