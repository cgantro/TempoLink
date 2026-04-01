#include "tempolink/juce/network/http/room/RoomApiPayloadBuilder.h"

#include <nlohmann/json.hpp>

#include "tempolink/juce/network/http/common/HttpTransport.h"

namespace tempolink::juceapp::network::roomapi {

namespace {

using nlohmann::json;
namespace http = tempolink::juceapp::network::http;

}  // namespace

std::string BuildRoomListPath(const RoomListFilter& filter) {
  std::vector<std::string> query_params;
  if (!filter.query.empty()) {
    query_params.push_back("q=" + http::EncodeQueryValue(filter.query));
  }
  if (!filter.tag.empty()) {
    query_params.push_back("tag=" + http::EncodeQueryValue(filter.tag));
  }
  if (filter.is_public.has_value()) {
    query_params.push_back(std::string("isPublic=") +
                           (filter.is_public.value() ? "true" : "false"));
  }
  if (filter.has_password.has_value()) {
    query_params.push_back(std::string("hasPassword=") +
                           (filter.has_password.value() ? "true" : "false"));
  }
  if (!filter.mode.empty()) {
    query_params.push_back("mode=" + http::EncodeQueryValue(filter.mode));
  }
  if (query_params.empty()) {
    return "/api/rooms";
  }

  std::string url = "/api/rooms?";
  for (std::size_t i = 0; i < query_params.size(); ++i) {
    if (i > 0) {
      url += "&";
    }
    url += query_params[i];
  }
  return url;
}

juce::String BuildUserActionBody(const std::string& user_id) {
  const json payload = {
      {"userId", user_id},
  };
  return juce::String(payload.dump());
}

juce::String BuildCreateRoomBody(const std::string& host_user_id,
                                 int max_participants) {
  const int clamped_max = juce::jlimit(2, 6, max_participants <= 0 ? 6 : max_participants);
  const json payload = {
      {"hostUserId", host_user_id},
      {"maxParticipants", clamped_max},
  };
  return juce::String(payload.dump());
}

juce::String BuildUpdateRoomBody(const std::string& host_user_id,
                                 const RoomUpdatePayload& payload) {
  json tags = json::array();
  for (const auto& tag : payload.tags) {
    const auto trimmed = tag.trim();
    if (trimmed.isNotEmpty()) {
      tags.push_back(trimmed.toStdString());
    }
  }

  const json root = {
      {"hostUserId", host_user_id},
      {"name", payload.name.toStdString()},
      {"description", payload.description.toStdString()},
      {"tags", tags},
      {"isPublic", payload.is_public},
      {"hasPassword", payload.has_password},
      {"password", payload.has_password ? payload.password.toStdString() : ""},
      {"maxParticipants", juce::jlimit(2, 6, payload.max_participants)},
  };
  return juce::String(root.dump());
}

juce::String BuildDeleteRoomBody(const std::string& host_user_id) {
  const json payload = {
      {"hostUserId", host_user_id},
  };
  return juce::String(payload.dump());
}

}  // namespace tempolink::juceapp::network::roomapi

