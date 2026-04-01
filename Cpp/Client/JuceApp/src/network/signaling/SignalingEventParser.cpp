#include "tempolink/juce/network/signaling/SignalingEventParser.h"

#include <nlohmann/json.hpp>

#include "tempolink/juce/constants/ClientText.h"
#include "tempolink/juce/network/http/common/JsonParse.h"

namespace tempolink::juceapp::network {

namespace {

using nlohmann::json;
namespace jp = tempolink::juceapp::jsonparse;

std::vector<std::string> ParseParticipants(const json& payload) {
  std::vector<std::string> participants;
  const auto it = payload.find("participants");
  if (it == payload.end() || !it->is_array()) {
    return participants;
  }
  participants.reserve(it->size());
  for (const auto& item : *it) {
    if (item.is_string()) {
      participants.push_back(item.get<std::string>());
    }
  }
  return participants;
}

}  // namespace

std::optional<SignalingClient::Event> ParseSignalingEvent(const juce::String& message_text) {
  const json root = jp::Parse(message_text);
  if (root.is_discarded() || !root.is_object()) {
    return std::nullopt;
  }

  const std::string type = jp::String(root, "type", "");
  if (type.empty()) {
    return std::nullopt;
  }
  const std::string room_code = jp::String(root, "roomCode", "");
  const std::string from_user_id = jp::String(root, "fromUserId", "");
  const std::string to_user_id = jp::String(root, "toUserId", "");
  const json payload =
      root.contains("payload") && root["payload"].is_object() ? root["payload"] : json::object();

  SignalingClient::Event event;
  event.room_code = room_code;
  event.from_user_id = from_user_id;
  event.to_user_id = to_user_id;

  if (type == "room.joined") {
    event.type = SignalingClient::Event::Type::RoomJoined;
    event.participants = ParseParticipants(payload);
    return event;
  }

  if (type == "peer.joined") {
    event.type = SignalingClient::Event::Type::PeerJoined;
    event.user_id = jp::String(payload, "userId", "");
    return event;
  }

  if (type == "peer.left") {
    event.type = SignalingClient::Event::Type::PeerLeft;
    event.user_id = jp::String(payload, "userId", "");
    return event;
  }

  if (type == "peer.ping") {
    event.type = SignalingClient::Event::Type::PeerPing;
    event.sent_at_ms = static_cast<std::uint64_t>(
        jsonparse::Int64(payload, "sentAtMs", 0));
    return event;
  }

  if (type == "peer.pong") {
    event.type = SignalingClient::Event::Type::PeerPong;
    event.sent_at_ms = static_cast<std::uint64_t>(
        jsonparse::Int64(payload, "sentAtMs", 0));
    return event;
  }

  if (type == "signal.error") {
    event.type = SignalingClient::Event::Type::Error;
    event.message = juce::String(jp::String(payload, "message",
                                             tempolink::juceapp::text::kSignalingErrorFallback));
    return event;
  }

  return std::nullopt;
}

}  // namespace tempolink::juceapp::network
