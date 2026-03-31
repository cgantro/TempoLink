#include "tempolink/juce/network/RoomApiClient.h"

#include <thread>
#include <utility>

#include <juce_events/juce_events.h>

namespace {

juce::URL BuildUrl(const std::string& base_url, const std::string& path) {
  return juce::URL(juce::String(base_url + path));
}

juce::String BuildJsonBody(const std::string& user_id) {
  juce::String body;
  body << "{\"userId\":\"" << juce::String(user_id) << "\"}";
  return body;
}

std::string TrimTrailingSlash(std::string value) {
  while (!value.empty() && value.back() == '/') {
    value.pop_back();
  }
  return value;
}

std::string EscapePathSegment(const std::string& value) {
  return juce::URL::addEscapeChars(juce::String(value), false).toStdString();
}

}  // namespace

RoomApiClient::RoomApiClient(std::string base_url)
    : base_url_(TrimTrailingSlash(std::move(base_url))) {}

void RoomApiClient::fetchRoomsAsync(RoomsCallback callback) const {
  std::thread([base_url = base_url_, callback = std::move(callback)]() mutable {
    juce::String error_text;
    std::vector<RoomSummary> rooms;

    const juce::URL url = BuildUrl(base_url, "/api/rooms");
    int status = 0;
    auto stream = url.createInputStream(juce::URL::InputStreamOptions(
                                            juce::URL::ParameterHandling::inAddress)
                                            .withConnectionTimeoutMs(3500)
                                            .withStatusCode(&status)
                                            .withHttpRequestCmd("GET"));
    if (stream == nullptr) {
      error_text = "Failed to connect room API";
    } else {
      const juce::String body = stream->readEntireStreamAsString();
      if (status >= 200 && status < 300) {
        rooms = parseRooms(body);
      } else {
        error_text = "Room API returned HTTP " + juce::String(status);
      }
    }

    juce::MessageManager::callAsync([callback = std::move(callback), rooms = std::move(rooms),
                                     error_text]() mutable {
      callback(error_text.isEmpty(), std::move(rooms), error_text);
    });
  }).detach();
}

void RoomApiClient::joinRoomAsync(const std::string& room_code,
                                  const std::string& user_id,
                                  SimpleCallback callback) const {
  std::thread([base_url = base_url_, room_code, user_id, callback = std::move(callback)]() mutable {
    juce::String error_text;
    const bool ok = postUserAction(
        base_url + "/api/rooms/" + EscapePathSegment(room_code) + "/join", user_id,
                                   error_text);
    juce::MessageManager::callAsync([callback = std::move(callback), ok, error_text]() mutable {
      callback(ok, error_text);
    });
  }).detach();
}

void RoomApiClient::leaveRoomAsync(const std::string& room_code,
                                   const std::string& user_id,
                                   SimpleCallback callback) const {
  std::thread([base_url = base_url_, room_code, user_id, callback = std::move(callback)]() mutable {
    juce::String error_text;
    const bool ok = postUserAction(
        base_url + "/api/rooms/" + EscapePathSegment(room_code) + "/leave", user_id,
        error_text);
    juce::MessageManager::callAsync([callback = std::move(callback), ok, error_text]() mutable {
      callback(ok, error_text);
    });
  }).detach();
}

bool RoomApiClient::postUserAction(const std::string& endpoint,
                                   const std::string& user_id,
                                   juce::String& error_text) {
  juce::URL url(endpoint);
  url = url.withPOSTData(BuildJsonBody(user_id));

  int status = 0;
  auto stream = url.createInputStream(
      juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
          .withConnectionTimeoutMs(3500)
          .withStatusCode(&status)
          .withHttpRequestCmd("POST")
          .withExtraHeaders("Content-Type: application/json\r\nAccept: application/json\r\n"));
  if (stream == nullptr) {
    error_text = "Failed to call room action API";
    return false;
  }

  (void)stream->readEntireStreamAsString();
  if (status < 200 || status >= 300) {
    error_text = "Room action API returned HTTP " + juce::String(status);
    return false;
  }
  return true;
}

std::vector<RoomSummary> RoomApiClient::parseRooms(const juce::String& json_text) {
  std::vector<RoomSummary> rooms;
  juce::var root = juce::JSON::parse(json_text);
  if (!root.isArray()) {
    return rooms;
  }

  const auto* arr = root.getArray();
  if (arr == nullptr) {
    return rooms;
  }

  rooms.reserve(static_cast<std::size_t>(arr->size()));
  for (const auto& entry : *arr) {
    auto* obj = entry.getDynamicObject();
    if (obj == nullptr) {
      continue;
    }

    RoomSummary room;
    room.room_code = obj->getProperty("roomCode").toString().toStdString();
    room.host_user_id = obj->getProperty("hostUserId").toString().toStdString();
    room.host_label = obj->getProperty("hostUserId").toString();
    room.relay_host = obj->getProperty("relayHost").toString().toStdString();
    room.relay_port = static_cast<std::uint16_t>(juce::jlimit(
        1, 65535, obj->getProperty("relayPort").toString().getIntValue()));
    room.members = obj->getProperty("participantCount").toString().getIntValue();
    room.capacity = obj->getProperty("maxParticipants").toString().getIntValue();
    room.title = "Room " + obj->getProperty("roomCode").toString();
    room.genre = "Live Jam";
    room.latency_hint = "20-50ms";

    const auto participants_var = obj->getProperty("participants");
    if (participants_var.isArray()) {
      if (const auto* p_arr = participants_var.getArray(); p_arr != nullptr) {
        for (const auto& p : *p_arr) {
          room.participants.push_back(p.toString().toStdString());
        }
      }
    }

    if (!room.room_code.empty()) {
      rooms.push_back(std::move(room));
    }
  }
  return rooms;
}
