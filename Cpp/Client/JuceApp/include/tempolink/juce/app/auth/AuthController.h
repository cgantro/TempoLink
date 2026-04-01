#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

#include "tempolink/juce/network/auth/AuthApiClient.h"
#include "tempolink/juce/network/auth/OAuthCallbackServer.h"
#include "tempolink/juce/ui/screens/LoginView.h"

class AuthController {
 public:
  AuthController(AuthApiClient& auth_api, OAuthCallbackServer& callback_server,
                 LoginView& login_view,
                 std::shared_ptr<std::atomic_bool> alive_flag,
                 const std::string& callback_host,
                 const std::uint16_t& callback_port,
                 const std::string& callback_path);

  void Initialize(
      const std::function<void(const SocialAuthSession&)>& on_auth_ready);
  void Shutdown();
  void Tick();

  void RefreshAuthProviders();
  void StartOAuthLogin(const std::string& provider);

 private:
  void HandleOAuthCallback(const std::string& ticket,
                           const std::string& provider,
                           const std::string& error,
                           const std::string& message);

  AuthApiClient& auth_api_;
  OAuthCallbackServer& callback_server_;
  LoginView& login_view_;
  std::shared_ptr<std::atomic_bool> alive_flag_;
  const std::string& callback_host_;
  const std::uint16_t& callback_port_;
  const std::string& callback_path_;

  std::function<void(const SocialAuthSession&)> on_auth_ready_;
  std::string oauth_redirect_uri_;
  bool oauth_login_pending_ = false;
  std::uint32_t oauth_login_started_ms_ = 0;
  std::string oauth_pending_provider_;
};
