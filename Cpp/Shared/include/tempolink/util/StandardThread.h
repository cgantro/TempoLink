#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

namespace tempolink::util {

/**
 * A minimal polyfill for std::stop_token functionality on platforms 
 * where C++20 jthread is not fully available (e.g., macOS Apple Clang).
 */
class StopToken {
 public:
  StopToken() = default;
  explicit StopToken(std::shared_ptr<std::atomic<bool>> stop_requested)
      : stop_requested_(stop_requested) {}

  [[nodiscard]] bool stop_requested() const noexcept {
    return stop_requested_ && stop_requested_->load(std::memory_order_acquire);
  }

 private:
  std::shared_ptr<std::atomic<bool>> stop_requested_;
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
    stop_requested_ = std::make_shared<std::atomic<bool>>(false);

    thread_ = std::thread([stop = stop_requested_,
                           f = std::forward<Callable>(f), 
                           ... args = std::forward<Args>(args)]() mutable {
      f(StopToken(std::move(stop)), std::forward<Args>(args)...);
    });
  }

  ~StandardThread() {
    if (thread_.joinable()) {
      request_stop();
      thread_.join();
    }
  }

  // Move-only like std::thread
  StandardThread(StandardThread&& other) noexcept
      : stop_requested_(std::move(other.stop_requested_)),
        thread_(std::move(other.thread_)) {}

  StandardThread& operator=(StandardThread&& other) noexcept {
    if (this != &other) {
      if (thread_.joinable()) {
        request_stop();
        thread_.join();
      }
      stop_requested_ = std::move(other.stop_requested_);
      thread_ = std::move(other.thread_);
    }
    return *this;
  }

  StandardThread(const StandardThread&) = delete;
  StandardThread& operator=(const StandardThread&) = delete;

  void request_stop() noexcept {
    if (stop_requested_) {
      stop_requested_->store(true, std::memory_order_release);
    }
  }

  void join() { thread_.join(); }
  bool joinable() const noexcept { return thread_.joinable(); }
  void detach() { thread_.detach(); }

 private:
  std::shared_ptr<std::atomic<bool>> stop_requested_;
  std::thread thread_;
};

}  // namespace tempolink::util
