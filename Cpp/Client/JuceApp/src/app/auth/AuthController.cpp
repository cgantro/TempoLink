#include "tempolink/juce/app/auth/AuthController.h"

#include <algorithm>
#include <vector>

#include <juce_core/juce_core.h>

#include "tempolink/juce/logging/AppLogger.h"

namespace {
constexpr std::uint32_t kOAuthLoginTimeoutMs = 90U * 1000U;
}

AuthController::AuthController(AuthApiClient& auth_api,
                               OAuthCallbackServer& callback_server,
                               LoginView& login_view,
                               std::shared_ptr<std::atomic_bool> alive_flag,
                               const std::string& callback_host,
                               const std::uint16_t& callback_port,
                               const std::string& callback_path)
    : auth_api_(auth_api),
      callback_server_(callback_server),
      login_view_(login_view),
      alive_flag_(std::move(alive_flag)),
      callback_host_(callback_host),
      callback_port_(callback_port),
      callback_path_(callback_path) {}

void AuthController::Initialize(
    const std::function<void(const SocialAuthSession&)>& on_auth_ready) {
  on_auth_ready_ = on_auth_ready;

  const bool callback_started = callback_server_.start(
      callback_port_,
      [this, alive = alive_flag_](const std::string& ticket,
                                  const std::string& provider,
                                  const std::string& error,
                                  const std::string& message) {
        if (!alive->load()) {
          return;
        }
        HandleOAuthCallback(ticket, provider, error, message);
      });

  if (callback_started) {
    oauth_redirect_uri_ = std::string("http://") + callback_host_ + ":" +
                          std::to_string(callback_server_.port()) +
                          callback_path_;
  } else {
    oauth_redirect_uri_.clear();
  }

  login_view_.setStatusText(callback_started
                                ? "Continue with Google login."
                                : "OAuth callback server failed to start.");
  RefreshAuthProviders();
}

void AuthController::Shutdown() { callback_server_.stop(); }

void AuthController::Tick() {
  if (!oauth_login_pending_) {
    return;
  }

  const auto now_ms = juce::Time::getMillisecondCounter();
  if (now_ms - oauth_login_started_ms_ < kOAuthLoginTimeoutMs) {
    return;
  }

  oauth_login_pending_ = false;
  oauth_pending_provider_.clear();
  login_view_.setBusy(false);
  login_view_.setStatusText(
      "Login cancelled or timed out. Click Google login to try again.");
}

void AuthController::RefreshAuthProviders() {
  login_view_.setBusy(true);
  login_view_.setStatusText("Loading OAuth providers...");
  auto alive = alive_flag_;
  auth_api_.fetchProvidersAsync(
      [this, alive](bool ok, std::vector<AuthProviderInfo> providers,
                    juce::String error_text) {
        if (!alive->load()) {
          return;
        }
        login_view_.setBusy(false);
        if (!ok) {
          login_view_.setStatusText("Failed to load providers: " + error_text);
          login_view_.setProviders({});
          return;
        }

        login_view_.setProviders(providers);
        const bool has_enabled = std::any_of(
            providers.begin(), providers.end(),
            [](const AuthProviderInfo& provider) { return provider.enabled; });
        if (has_enabled) {
          login_view_.setStatusText(
              "Providers loaded. Continue with social login.");
        } else {
          login_view_.setStatusText("No OAuth provider configured on server.");
        }
      });
}

void AuthController::StartOAuthLogin(const std::string& provider) {
  if (oauth_redirect_uri_.empty()) {
    login_view_.setBusy(false);
    login_view_.setStatusText("OAuth callback server is unavailable.");
    return;
  }

  login_view_.setStatusText("Opening browser for " + juce::String(provider) +
                            " login...");
  const auto start_url = auth_api_.buildStartUrl(provider, oauth_redirect_uri_);
  tempolink::juceapp::logging::Info(
      "OAuth start provider=" + juce::String(provider) +
      " redirectUri=" + juce::String(oauth_redirect_uri_) +
      " startUrl=" + juce::String(start_url));
  const bool launched = juce::URL(start_url).launchInDefaultBrowser();
  if (!launched) {
    oauth_login_pending_ = false;
    oauth_pending_provider_.clear();
    login_view_.setBusy(false);
    login_view_.setStatusText("Failed to launch browser.");
    return;
  }

  oauth_login_pending_ = true;
  oauth_login_started_ms_ = juce::Time::getMillisecondCounter();
  oauth_pending_provider_ = provider;
  login_view_.setBusy(false);
  login_view_.setStatusText(
      "Browser opened. Complete Google login, or click login again if you "
      "closed it.");
}

void AuthController::HandleOAuthCallback(const std::string& ticket,
                                         const std::string& provider,
                                         const std::string& error,
                                         const std::string& message) {
  oauth_login_pending_ = false;
  oauth_pending_provider_.clear();

  if (!error.empty()) {
    login_view_.setBusy(false);
    login_view_.setStatusText("Login failed (" + juce::String(provider) +
                              "): " + juce::String(message));
    return;
  }

  if (ticket.empty()) {
    login_view_.setBusy(false);
    login_view_.setStatusText("OAuth callback missing ticket.");
    return;
  }

  login_view_.setBusy(true);
  login_view_.setStatusText("Exchanging login ticket...");
  auto alive = alive_flag_;
  auth_api_.exchangeTicketAsync(
      ticket, [this, alive](bool ok, SocialAuthSession session,
                            juce::String error_text) {
        if (!alive->load()) {
          return;
        }
        login_view_.setBusy(false);
        if (!ok) {
          login_view_.setStatusText("Ticket exchange failed: " + error_text);
          return;
        }
        if (on_auth_ready_) {
          on_auth_ready_(session);
        }
      });
}
