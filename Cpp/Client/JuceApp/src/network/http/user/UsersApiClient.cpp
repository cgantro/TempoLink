#include "tempolink/juce/network/http/user/UsersApiClient.h"

#include <thread>
#include <utility>

#include <juce_events/juce_events.h>

#include "tempolink/juce/constants/ApiPaths.h"
#include "tempolink/juce/network/http/common/HttpTransport.h"
#include "tempolink/juce/network/http/common/JsonParse.h"

namespace {

namespace http = tempolink::juceapp::network::http;
namespace jsonparse = tempolink::juceapp::jsonparse;

std::vector<UserSummary> ParseUsers(const juce::String& text) {
  std::vector<UserSummary> users;
  const auto json = jsonparse::Parse(text);
  const auto* list = &json;
  if (json.is_object()) {
    if (const auto it = json.find("users"); it != json.end() && it->is_array()) {
      list = &(*it);
    } else if (const auto it = json.find("items");
               it != json.end() && it->is_array()) {
      list = &(*it);
    } else if (const auto it = json.find("content");
               it != json.end() && it->is_array()) {
      list = &(*it);
    }
  }
  if (!list->is_array()) {
    return users;
  }

  users.reserve(list->size());
  for (const auto& item : *list) {
    if (!item.is_object()) {
      continue;
    }
    UserSummary user;
    user.user_id = jsonparse::String(item, "userId", "");
    user.display_name = jsonparse::String(item, "displayName",
                                          jsonparse::String(item, "nickname", ""));
    user.bio = jsonparse::String(item, "bio", "");
    if (user.display_name.isEmpty()) {
      user.display_name = "Unknown";
    }
    users.push_back(std::move(user));
  }
  return users;
}

}  // namespace

UsersApiClient::UsersApiClient(std::string base_url)
    : base_url_(http::NormalizeBaseUrl(std::move(base_url))) {}

void UsersApiClient::fetchUsersAsync(const std::string& query, Callback callback) const {
  std::thread([base_url = base_url_, query, callback = std::move(callback)]() mutable {
    juce::String error_text;
    std::vector<UserSummary> users;

    http::Request request;
    request.method = "GET";
    request.path = tempolink::juceapp::constants::kUsersPath;
    if (!query.empty()) {
      request.path += "?q=" + http::EncodeQueryValue(query);
    }
    request.timeout_ms = 3500;

    const auto response = http::Perform(base_url, request);
    if (!response.transport_ok) {
      error_text = "Users API call failed.";
    } else if (response.isHttpSuccess()) {
      users = ParseUsers(response.body);
    } else {
      error_text = "Users API HTTP " + juce::String(response.status_code);
    }

    juce::MessageManager::callAsync([callback = std::move(callback), users = std::move(users),
                                     error_text]() mutable {
      callback(error_text.isEmpty(), std::move(users), error_text);
    });
  }).detach();
}
