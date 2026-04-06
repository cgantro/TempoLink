#include "tempolink/juce/network/http/room/RoomApiJsonMapper.h"

#include <nlohmann/json.hpp>

#include "tempolink/juce/network/http/common/JsonParse.h"

namespace tempolink::juceapp::network::roomapi {

namespace {

using nlohmann::json;
namespace jp = tempolink::juceapp::jsonparse;

inline constexpr char kTestRoomCode[] = "TEST00";
inline constexpr char kTestRoomTitle[] = "TestRoom";
inline constexpr char kTestRoomApiLatencyHint[] = "Audio/Network Test";

RoomSummary ParseRoomObject(const json& obj) {
  RoomSummary room;
  room.room_code = jp::String(obj, "roomCode", "");
  room.host_user_id = jp::String(obj, "hostUserId", "");
  room.host_label = juce::String(room.host_user_id);
  room.title = juce::String(jp::String(obj, "name", "")).trim();
  room.description = juce::String(jp::String(obj, "description", ""));
  room.relay_host = jp::String(obj, "relayHost", "");
  const int relay_port = jp::Int(obj, "relayPort", 0);
  room.relay_port =
      relay_port > 0 && relay_port <= 65535 ? static_cast<std::uint16_t>(relay_port) : 0;
  room.is_public = jp::Bool(obj, "isPublic", true);
  room.has_password = jp::Bool(obj, "hasPassword", false);
  room.members = jp::Int(obj, "participantCount", 0);
  room.capacity = jp::Int(obj, "maxParticipants", 0);
  room.is_active = room.members > 0;
  const auto room_code = juce::String(room.room_code);
  if (room.title.isEmpty()) {
    room.title = room_code == kTestRoomCode
                     ? juce::String(kTestRoomTitle)
                     : juce::String("Room ") + room_code;
  }
  room.genre = room.is_public ? "Public" : "Private";
  room.latency_hint =
      room_code == kTestRoomCode
          ? juce::String(kTestRoomApiLatencyHint)
          : juce::String("20-50ms");

  if (obj.contains("tags") && obj["tags"].is_array()) {
    for (const auto& tag : obj["tags"]) {
      if (tag.is_string()) {
        room.tags.push_back(juce::String(tag.get<std::string>()));
      }
    }
  }

  if (obj.contains("participants") && obj["participants"].is_array()) {
    for (const auto& p : obj["participants"]) {
      if (p.is_string()) {
        room.participants.push_back(p.get<std::string>());
      }
    }
  }

  return room;
}

}  // namespace

bool ParseCreatedRoom(const juce::String& json_text, RoomSummary& room) {
  const json root = jp::Parse(json_text);
  if (root.is_discarded() || !root.is_object()) {
    return false;
  }
  if (root.contains("roomCode")) {
    room = ParseRoomObject(root);
    return !room.room_code.empty();
  }
  if (root.contains("room") && root["room"].is_object()) {
    room = ParseRoomObject(root["room"]);
    return !room.room_code.empty();
  }
  return false;
}

std::vector<RoomSummary> ParseRoomList(const juce::String& json_text) {
  std::vector<RoomSummary> rooms;
  const json root = jp::Parse(json_text);
  if (root.is_discarded() || !root.is_array()) {
    return rooms;
  }

  rooms.reserve(root.size());
  for (const auto& entry : root) {
    if (!entry.is_object()) {
      continue;
    }
    RoomSummary room = ParseRoomObject(entry);
    if (!room.room_code.empty()) {
      rooms.push_back(std::move(room));
    }
  }
  return rooms;
}

}  // namespace tempolink::juceapp::network::roomapi
