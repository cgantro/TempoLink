#include "tempolink/juce/network/signaling/SignalingEventParser.h"

#include <functional>
#include <string>
#include <unordered_map>

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

// --- Individual event handlers ---

using ParseContext = std::pair<const json& /*payload*/, SignalingClient::Event& /*event*/>;
using EventHandler = std::function<void(const json& payload, SignalingClient::Event& event)>;

void HandleRoomJoined(const json& payload, SignalingClient::Event& event) {
  event.type = SignalingClient::Event::Type::RoomJoined;
  event.participants = ParseParticipants(payload);
}

void HandlePeerJoined(const json& payload, SignalingClient::Event& event) {
  event.type = SignalingClient::Event::Type::PeerJoined;
  event.user_id = jp::String(payload, "userId", "");
}

void HandlePeerLeft(const json& payload, SignalingClient::Event& event) {
  event.type = SignalingClient::Event::Type::PeerLeft;
  event.user_id = jp::String(payload, "userId", "");
}

void HandlePeerPing(const json& payload, SignalingClient::Event& event) {
  event.type = SignalingClient::Event::Type::PeerPing;
  event.sent_at_ms = static_cast<std::uint64_t>(
      jsonparse::Int64(payload, "sentAtMs", 0));
}

void HandlePeerPong(const json& payload, SignalingClient::Event& event) {
  event.type = SignalingClient::Event::Type::PeerPong;
  event.sent_at_ms = static_cast<std::uint64_t>(
      jsonparse::Int64(payload, "sentAtMs", 0));
}

void HandleChatMessage(const json& payload, SignalingClient::Event& event) {
  event.type = SignalingClient::Event::Type::ChatMessage;
  event.message = juce::String(jp::String(payload, "message", ""));
  event.sent_at_ms = static_cast<std::uint64_t>(
      jsonparse::Int64(payload, "sentAtMs", 0));
}

void HandleSignalError(const json& payload, SignalingClient::Event& event) {
  event.type = SignalingClient::Event::Type::Error;
  event.message = juce::String(jp::String(payload, "message",
                                           tempolink::juceapp::text::kSignalingErrorFallback));
}

/// Static dispatch map — built once, reused for all messages.
const std::unordered_map<std::string, EventHandler>& GetEventHandlers() {
  static const std::unordered_map<std::string, EventHandler> handlers = {
      {"room.joined",   HandleRoomJoined},
      {"peer.joined",   HandlePeerJoined},
      {"peer.left",     HandlePeerLeft},
      {"peer.ping",     HandlePeerPing},
      {"peer.pong",     HandlePeerPong},
      {"chat.message",  HandleChatMessage},
      {"signal.error",  HandleSignalError},
  };
  return handlers;
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

  const auto& handlers = GetEventHandlers();
  const auto it = handlers.find(type);
  if (it == handlers.end()) {
    return std::nullopt;  // Unknown event type
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

  it->second(payload, event);
  return event;
}

}  // namespace tempolink::juceapp::network
