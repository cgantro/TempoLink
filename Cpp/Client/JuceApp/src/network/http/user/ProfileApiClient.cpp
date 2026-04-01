#include "tempolink/juce/network/http/user/ProfileApiClient.h"

#include <thread>
#include <utility>

#include <juce_events/juce_events.h>
#include <nlohmann/json.hpp>

#include "tempolink/juce/constants/ApiPaths.h"
#include "tempolink/juce/network/http/common/HttpTransport.h"
#include "tempolink/juce/network/http/common/JsonParse.h"

namespace {

namespace http = tempolink::juceapp::network::http;
namespace jsonparse = tempolink::juceapp::jsonparse;

UserProfileModel ParseProfile(const juce::String& body) {
  UserProfileModel profile;
  const auto json = jsonparse::Parse(body);
  if (!json.is_object()) {
    return profile;
  }
  profile.user_id = jsonparse::String(json, "userId", "");
  profile.display_name = jsonparse::String(json, "displayName", "");
  profile.bio = jsonparse::String(json, "bio", "");
  profile.email = jsonparse::String(json, "email", "");
  profile.avatar_url = jsonparse::String(json, "avatarUrl", "");
  profile.provider = jsonparse::String(json, "provider", "");
  return profile;
}

}  // namespace

ProfileApiClient::ProfileApiClient(std::string base_url)
    : base_url_(http::NormalizeBaseUrl(std::move(base_url))) {}

void ProfileApiClient::fetchProfileAsync(const std::string& user_id,
                                         FetchCallback callback) const {
  std::thread([base_url = base_url_, user_id, callback = std::move(callback)]() mutable {
    juce::String error_text;
    UserProfileModel profile;

    if (user_id.empty()) {
      error_text = "Missing userId";
      juce::MessageManager::callAsync([callback = std::move(callback), profile, error_text]() mutable {
        callback(false, profile, error_text);
      });
      return;
    }

    http::Request request;
    request.method = "GET";
    request.path = std::string(tempolink::juceapp::constants::kProfileMePath) +
                   "?userId=" + http::EncodeQueryValue(user_id);
    request.timeout_ms = 3500;

    const auto response = http::Perform(base_url, request);
    if (!response.transport_ok) {
      error_text = "Profile API call failed.";
    } else if (response.isHttpSuccess()) {
      profile = ParseProfile(response.body);
      if (profile.user_id.isEmpty()) {
        error_text = "Profile response is missing userId.";
      }
    } else {
      error_text = "Profile API HTTP " + juce::String(response.status_code);
    }

    juce::MessageManager::callAsync([callback = std::move(callback), profile, error_text]() mutable {
      callback(error_text.isEmpty(), profile, error_text);
    });
  }).detach();
}

void ProfileApiClient::updateProfileAsync(const std::string& user_id,
                                          const std::string& display_name,
                                          const std::string& bio,
                                          UpdateCallback callback) const {
  std::thread([base_url = base_url_, user_id, display_name, bio,
               callback = std::move(callback)]() mutable {
    juce::String error_text;
    UserProfileModel profile;

    if (user_id.empty()) {
      error_text = "Missing userId";
      juce::MessageManager::callAsync([callback = std::move(callback), profile, error_text]() mutable {
        callback(false, profile, error_text);
      });
      return;
    }

    nlohmann::json payload = {
        {"userId", user_id},
        {"displayName", display_name},
        {"bio", bio},
    };

    http::Request request;
    request.method = "PUT";
    request.path = tempolink::juceapp::constants::kProfileMePath;
    request.body = juce::String(payload.dump());
    request.timeout_ms = 4000;

    const auto response = http::Perform(base_url, request);
    if (!response.transport_ok) {
      error_text = "Profile update request failed.";
    } else if (response.isHttpSuccess()) {
      profile = ParseProfile(response.body);
      if (profile.user_id.isEmpty()) {
        error_text = "Profile update response is missing userId.";
      }
    } else {
      error_text = "Profile update API HTTP " + juce::String(response.status_code);
    }

    juce::MessageManager::callAsync([callback = std::move(callback), profile, error_text]() mutable {
      callback(error_text.isEmpty(), profile, error_text);
    });
  }).detach();
}
