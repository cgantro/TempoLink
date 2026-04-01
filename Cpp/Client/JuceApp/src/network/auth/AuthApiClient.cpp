#include "tempolink/juce/network/auth/AuthApiClient.h"

#include <cctype>
#include <thread>
#include <utility>

#include <juce_events/juce_events.h>

#include "tempolink/juce/constants/ApiPaths.h"
#include "tempolink/juce/constants/AuthConstants.h"
#include "tempolink/juce/network/http/common/HttpTransport.h"
#include "tempolink/juce/network/http/common/JsonParse.h"

namespace {

namespace http = tempolink::juceapp::network::http;
namespace jsonparse = tempolink::juceapp::jsonparse;

std::vector<AuthProviderInfo> ParseProviders(const juce::String& text) {
  std::vector<AuthProviderInfo> providers;
  const auto json = jsonparse::Parse(text);
  if (!json.is_object()) {
    return providers;
  }
  const auto it = json.find("providers");
  if (it == json.end() || !it->is_array()) {
    return providers;
  }

  providers.reserve(it->size());
  for (const auto& item : *it) {
    if (!item.is_object()) {
      continue;
    }
    AuthProviderInfo provider;
    provider.id = jsonparse::String(item, "id", "");
    provider.enabled = jsonparse::Bool(item, "enabled", false);
    provider.start_url = jsonparse::String(item, "startUrl", "");
    if (!provider.id.empty()) {
      providers.push_back(std::move(provider));
    }
  }
  return providers;
}

SocialAuthSession ParseSession(const juce::String& text) {
  SocialAuthSession session;
  const auto json = jsonparse::Parse(text);
  if (!json.is_object()) {
    return session;
  }
  session.session_token = jsonparse::String(json, "sessionToken", "");
  session.user_id = jsonparse::String(json, "userId", "");
  session.provider = jsonparse::String(json, "provider", "");
  session.provider_user_id = jsonparse::String(json, "providerUserId", "");
  session.email = jsonparse::String(json, "email", "");
  session.display_name = jsonparse::String(json, "displayName", "");
  session.avatar_url = jsonparse::String(json, "avatarUrl", "");
  session.session_expires_at = jsonparse::String(json, "sessionExpiresAt", "");
  return session;
}

bool IsUnsafePort(int port) {
  return port <= 0 || port < 1024 || port == 1;
}

int ExtractPort(const std::string& uri) {
  const auto scheme_pos = uri.find("://");
  const auto host_start = scheme_pos == std::string::npos ? 0 : scheme_pos + 3;
  const auto path_pos = uri.find('/', host_start);
  const auto host_end = path_pos == std::string::npos ? uri.size() : path_pos;
  const auto colon_pos = uri.rfind(':', host_end);
  if (colon_pos == std::string::npos || colon_pos < host_start) {
    return -1;
  }
  int port = 0;
  for (std::size_t i = colon_pos + 1; i < host_end; ++i) {
    const unsigned char c = static_cast<unsigned char>(uri[i]);
    if (!std::isdigit(c)) {
      return -1;
    }
    port = (port * 10) + (uri[i] - '0');
    if (port > 65535) {
      return -1;
    }
  }
  return port;
}

std::string BuildSafeRedirectUri(const std::string& redirect_uri) {
  const std::string fallback = std::string("http://127.0.0.1:") +
                               std::to_string(tempolink::juceapp::constants::kOAuthCallbackPort) +
                               tempolink::juceapp::constants::kOAuthCallbackPath;
  if (redirect_uri.empty()) {
    return fallback;
  }
  const int port = ExtractPort(redirect_uri);
  if (IsUnsafePort(port)) {
    return fallback;
  }
  return redirect_uri;
}

}  // namespace

AuthApiClient::AuthApiClient(std::string base_url)
    : base_url_(http::NormalizeBaseUrl(std::move(base_url))) {}

void AuthApiClient::fetchProvidersAsync(ProvidersCallback callback) const {
  std::thread([base_url = base_url_, callback = std::move(callback)]() mutable {
    juce::String error_text;
    std::vector<AuthProviderInfo> providers;

    http::Request request;
    request.method = "GET";
    request.path = tempolink::juceapp::constants::kAuthProvidersPath;
    request.timeout_ms = 3500;

    const auto response = http::Perform(base_url, request);
    if (!response.transport_ok) {
      error_text = response.error_text.isNotEmpty() ? response.error_text
                                                    : "Auth providers API call failed.";
    } else if (response.isHttpSuccess()) {
      providers = ParseProviders(response.body);
    } else {
      error_text = "Auth providers API HTTP " + juce::String(response.status_code);
    }

    juce::MessageManager::callAsync(
        [callback = std::move(callback), providers = std::move(providers),
         error_text]() mutable { callback(error_text.isEmpty(), std::move(providers), error_text); });
  }).detach();
}

void AuthApiClient::exchangeTicketAsync(const std::string& ticket,
                                        ExchangeCallback callback) const {
  std::thread([base_url = base_url_, ticket, callback = std::move(callback)]() mutable {
    juce::String error_text;
    SocialAuthSession session;

    if (ticket.empty()) {
      error_text = "Empty OAuth ticket";
      juce::MessageManager::callAsync(
          [callback = std::move(callback), session, error_text]() mutable {
            callback(false, session, error_text);
          });
      return;
    }

    http::Request request;
    request.method = "GET";
    request.path = std::string(tempolink::juceapp::constants::kAuthTicketPrefix) +
                   http::EscapePathSegment(ticket);
    request.timeout_ms = 3500;

    const auto response = http::Perform(base_url, request);
    if (!response.transport_ok) {
      error_text = response.error_text.isNotEmpty() ? response.error_text
                                                    : "Ticket exchange request failed.";
    } else if (response.isHttpSuccess()) {
      session = ParseSession(response.body);
      if (session.session_token.isEmpty()) {
        error_text = "Ticket exchange returned empty session token.";
      }
    } else {
      error_text = "Ticket exchange API HTTP " + juce::String(response.status_code);
    }

    juce::MessageManager::callAsync(
        [callback = std::move(callback), session, error_text]() mutable {
          callback(error_text.isEmpty(), session, error_text);
        });
  }).detach();
}

std::string AuthApiClient::buildStartUrl(const std::string& provider,
                                         const std::string& redirect_uri) const {
  const auto safe_redirect_uri = BuildSafeRedirectUri(redirect_uri);
  std::string url = base_url_ + std::string(tempolink::juceapp::constants::kAuthSocialStartPrefix) +
                    http::EscapePathSegment(provider) + "/start";
  if (!safe_redirect_uri.empty()) {
    url += "?redirectUri=" + http::EncodeQueryValue(safe_redirect_uri);
  }
  return url;
}
