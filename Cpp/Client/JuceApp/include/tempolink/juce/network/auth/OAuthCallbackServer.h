#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <thread>

namespace httplib {
class Server;
}

class OAuthCallbackServer {
 public:
  using Callback = std::function<void(const std::string& ticket,
                                      const std::string& provider,
                                      const std::string& error,
                                      const std::string& message)>;

  OAuthCallbackServer();
  ~OAuthCallbackServer();

  bool start(std::uint16_t port, Callback callback);
  void stop();

  bool isRunning() const;
  std::uint16_t port() const;

 private:
  std::atomic_bool running_{false};
  std::uint16_t port_{0};
  Callback callback_;
  std::unique_ptr<httplib::Server> server_;
  std::thread server_thread_;
};
