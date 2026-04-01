#include "tempolink/juce/network/auth/OAuthCallbackServer.h"

#include <utility>

#include <httplib.h>
#include <juce_events/juce_events.h>

namespace {

std::string GetParam(const httplib::Request& req, const std::string& key) {
  return req.has_param(key) ? req.get_param_value(key) : std::string{};
}

}  // namespace

OAuthCallbackServer::OAuthCallbackServer() = default;

OAuthCallbackServer::~OAuthCallbackServer() { stop(); }

bool OAuthCallbackServer::start(std::uint16_t port, Callback callback) {
  stop();

  port_ = port;
  callback_ = std::move(callback);
  server_ = std::make_unique<httplib::Server>();

  server_->Get("/oauth/callback", [this](const httplib::Request& req, httplib::Response& res) {
    const std::string ticket = GetParam(req, "ticket");
    const std::string provider = GetParam(req, "provider");
    const std::string error = GetParam(req, "error");
    const std::string message = GetParam(req, "message");

    res.set_content(
        "<html><body style='font-family:sans-serif'>"
        "<h3>TempoLink login callback received</h3>"
        "<p>You can return to the app.</p>"
        "</body></html>",
        "text/html");

    if (callback_) {
      auto cb = callback_;
      juce::MessageManager::callAsync([cb = std::move(cb), ticket, provider, error, message]() {
        cb(ticket, provider, error, message);
      });
    }
  });

  const int bound_port = server_->bind_to_port("127.0.0.1", static_cast<int>(port_));
  if (bound_port <= 0) {
    server_.reset();
    callback_ = nullptr;
    port_ = 0;
    return false;
  }

  port_ = static_cast<std::uint16_t>(bound_port);
  server_thread_ = std::thread([this] {
    running_.store(true, std::memory_order_release);
    server_->listen_after_bind();
    running_.store(false, std::memory_order_release);
  });

  return true;
}

void OAuthCallbackServer::stop() {
  if (server_) {
    server_->stop();
  }
  if (server_thread_.joinable()) {
    server_thread_.join();
  }
  server_.reset();
  callback_ = nullptr;
  running_.store(false, std::memory_order_release);
}

bool OAuthCallbackServer::isRunning() const {
  return running_.load(std::memory_order_acquire);
}

std::uint16_t OAuthCallbackServer::port() const { return port_; }
