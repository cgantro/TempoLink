#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

namespace tempolink::util {

/**
 * A minimal polyfill for std::stop_token functionality on platforms 
 * where C++20 jthread is not fully available (e.g., macOS Apple Clang).
 */
class StopToken {
 public:
  StopToken() : stop_requested_(nullptr) {}
  explicit StopToken(std::atomic<bool>* stop_requested)
      : stop_requested_(stop_requested) {}

  [[nodiscard]] bool stop_requested() const noexcept {
    return stop_requested_ && stop_requested_->load();
  }

 private:
  std::atomic<bool>* stop_requested_;
};

/**
 * A wrapper for std::thread that emulates std::jthread's cooperative 
 * cancellation and join-on-destruction behavior.
 */
class StandardThread {
 public:
  StandardThread() noexcept = default;

  template <typename Callable, typename... Args>
  explicit StandardThread(Callable&& f, Args&&... args) {
    stop_requested_ = std::make_unique<std::atomic<bool>>(false);
    
    // We pass a copy of the stop_requested pointer to the thread
    auto* stop_ptr = stop_requested_.get();
    
    thread_ = std::thread([stop_ptr, 
                           f = std::forward<Callable>(f), 
                           ... args = std::forward<Args>(args)]() mutable {
      f(StopToken(stop_ptr), std::forward<Args>(args)...);
    });
  }

  ~StandardThread() {
    if (thread_.joinable()) {
      request_stop();
      thread_.join();
    }
  }

  // Move-only like std::thread
  StandardThread(StandardThread&&) noexcept = default;
  StandardThread& operator=(StandardThread&&) noexcept = default;

  StandardThread(const StandardThread&) = delete;
  StandardThread& operator=(const StandardThread&) = delete;

  void request_stop() noexcept {
    if (stop_requested_) {
      stop_requested_->store(true);
    }
  }

  void join() { thread_.join(); }
  bool joinable() const noexcept { return thread_.joinable(); }
  void detach() { thread_.detach(); }

 private:
  std::unique_ptr<std::atomic<bool>> stop_requested_;
  std::thread thread_;
};

}  // namespace tempolink::util
