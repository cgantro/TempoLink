#include "tempolink/juce/network/signaling/SignalingClient.h"

#include <utility>

#include <juce_events/juce_events.h>
#include <nlohmann/json.hpp>

#include "tempolink/juce/constants/ClientText.h"
#include "tempolink/juce/logging/AppLogger.h"
#include "tempolink/juce/network/signaling/SignalingEventParser.h"
#include "tempolink/juce/network/transport/JuceWebSocketTransport.h"

namespace {

juce::String BuildWebSocketPath(const std::string& room_code,
                                const std::string& user_id) {
  const juce::String escaped_room =
      juce::URL::addEscapeChars(juce::String(room_code), true);
  const juce::String escaped_user =
      juce::URL::addEscapeChars(juce::String(user_id), true);
  return "/ws/signaling?roomCode=" + escaped_room + "&userId=" + escaped_user;
}

}  // namespace

using Transport = tempolink::juceapp::network::IWebSocketTransport;

SignalingClient::SignalingClient()
    : transport_(std::make_unique<tempolink::juceapp::network::JuceWebSocketTransport>()) {}

SignalingClient::SignalingClient(
    std::unique_ptr<tempolink::juceapp::network::IWebSocketTransport> transport)
    : transport_(std::move(transport)) {}

SignalingClient::~SignalingClient() { disconnect(); }

bool SignalingClient::connect(const std::string& host, int port,
                              const std::string& room_code,
                              const std::string& user_id,
                              EventCallback callback) {
  disconnect();
  {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    callback_ = std::move(callback);
  }

  room_code_ = room_code;
  user_id_ = user_id;

  transport_->SetOnFrameReceived([this](const Transport::Frame& frame) {
    onFrameReceived(frame);
  });
  transport_->SetOnDisconnected([this] { onTransportDisconnected(); });

  const juce::String path = BuildWebSocketPath(room_code, user_id);
  if (!transport_->Connect(host, port, path.toStdString())) {
    tempolink::juceapp::logging::Error("Signaling handshake failed");
    emitEvent(Event{Event::Type::Error, "", "", {},
                    tempolink::juceapp::text::kSignalingHandshakeFailed});
    return false;
  }

  return true;
}

void SignalingClient::disconnect() {
  transport_->Disconnect();
  room_code_.clear();
  user_id_.clear();
  std::lock_guard<std::mutex> lock(callback_mutex_);
  callback_ = {};
}

bool SignalingClient::isConnected() const { return transport_->IsConnected(); }

bool SignalingClient::sendPeerPing(const std::string& to_user_id,
                                   std::uint64_t sent_at_ms) {
  return sendEnvelope("peer.ping", to_user_id, sent_at_ms);
}

bool SignalingClient::sendPeerPong(const std::string& to_user_id,
                                   std::uint64_t sent_at_ms) {
  return sendEnvelope("peer.pong", to_user_id, sent_at_ms);
}

bool SignalingClient::sendChatMessage(const juce::String& text,
                                      const std::string& to_user_id) {
  if (!transport_->IsConnected()) return false;
  nlohmann::json payload = nlohmann::json::object();
  payload["message"] = text.toStdString();
  payload["sentAtMs"] = juce::Time::currentTimeMillis();

  nlohmann::json envelope = {
      {"type", "chat.message"},
      {"roomCode", room_code_},
      {"fromUserId", user_id_},
      {"toUserId", to_user_id},
      {"payload", payload},
  };
  const std::string serialized = envelope.dump();
  juce::MemoryBlock body(serialized.data(), serialized.size());
  return transport_->SendFrame(Transport::Opcode::Text, body);
}

void SignalingClient::onFrameReceived(const Transport::Frame& frame) {
  if (frame.opcode != Transport::Opcode::Text) return;
  const juce::String text(
      static_cast<const char*>(frame.payload.getData()),
      static_cast<int>(frame.payload.getSize()));
  dispatchTextMessage(text);
}

void SignalingClient::onTransportDisconnected() {
  // Transport layer disconnected — could emit an event to UI if needed.
}

void SignalingClient::dispatchTextMessage(const juce::String& message_text) {
  auto event = tempolink::juceapp::network::ParseSignalingEvent(message_text);
  if (!event.has_value()) return;
  emitEvent(std::move(*event));
}

void SignalingClient::emitEvent(Event event) {
  EventCallback callback;
  {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    callback = callback_;
  }
  if (!callback) return;
  juce::MessageManager::callAsync(
      [callback = std::move(callback), event = std::move(event)]() mutable {
        callback(event);
      });
}

bool SignalingClient::sendEnvelope(const juce::String& type,
                                   const std::string& to_user_id,
                                   std::optional<std::uint64_t> sent_at_ms) {
  if (!transport_->IsConnected()) return false;
  nlohmann::json payload = nlohmann::json::object();
  if (sent_at_ms.has_value()) payload["sentAtMs"] = *sent_at_ms;
  nlohmann::json envelope = {
      {"type", type.toStdString()},
      {"roomCode", room_code_},
      {"fromUserId", user_id_},
      {"toUserId", to_user_id},
      {"payload", payload},
  };
  const std::string serialized = envelope.dump();
  juce::MemoryBlock body(serialized.data(), serialized.size());
  return transport_->SendFrame(Transport::Opcode::Text, body);
}
