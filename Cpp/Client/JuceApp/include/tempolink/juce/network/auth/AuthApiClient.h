#pragma once

#include <functional>
#include <string>
#include <vector>

#include <juce_core/juce_core.h>

struct AuthProviderInfo {
  std::string id;
  bool enabled = false;
  juce::String start_url;
};

struct SocialAuthSession {
  juce::String session_token;
  juce::String user_id;
  juce::String provider;
  juce::String provider_user_id;
  juce::String email;
  juce::String display_name;
  juce::String avatar_url;
  juce::String session_expires_at;
};

class AuthApiClient {
 public:
  using ProvidersCallback =
      std::function<void(bool, std::vector<AuthProviderInfo>, juce::String)>;
  using ExchangeCallback =
      std::function<void(bool, SocialAuthSession, juce::String)>;

  explicit AuthApiClient(std::string base_url);

  void fetchProvidersAsync(ProvidersCallback callback) const;
  void exchangeTicketAsync(const std::string& ticket, ExchangeCallback callback) const;
  std::string buildStartUrl(const std::string& provider,
                            const std::string& redirect_uri) const;

 private:
  std::string base_url_;
};
