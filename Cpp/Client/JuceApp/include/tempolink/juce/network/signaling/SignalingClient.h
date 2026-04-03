#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include <juce_core/juce_core.h>

#include "tempolink/juce/network/transport/IWebSocketTransport.h"

class SignalingClient {
 public:
  struct Event {
    enum class Type { RoomJoined, PeerJoined, PeerLeft, PeerPing, PeerPong, ChatMessage, Error };
    Type type = Type::Error;
    std::string room_code;
    std::string from_user_id;
    std::string to_user_id;
    std::string user_id;
    std::uint64_t sent_at_ms = 0;
    std::vector<std::string> participants;
    juce::String message;
  };

  using EventCallback = std::function<void(const Event&)>;

  /// Construct with explicit transport (for DI / testing).
  explicit SignalingClient(
      std::unique_ptr<tempolink::juceapp::network::IWebSocketTransport> transport);

  /// Default constructor — creates JuceWebSocketTransport.
  SignalingClient();
  ~SignalingClient();

  bool connect(const std::string& host, int port, const std::string& room_code,
               const std::string& user_id, EventCallback callback);
  void disconnect();
  bool isConnected() const;
  bool sendPeerPing(const std::string& to_user_id, std::uint64_t sent_at_ms);
  bool sendPeerPong(const std::string& to_user_id, std::uint64_t sent_at_ms);
  bool sendChatMessage(const juce::String& text, const std::string& to_user_id = "");

 private:
  void onFrameReceived(
      const tempolink::juceapp::network::IWebSocketTransport::Frame& frame);
  void onTransportDisconnected();
  void dispatchTextMessage(const juce::String& message_text);
  void emitEvent(Event event);
  bool sendEnvelope(const juce::String& type,
                    const std::string& to_user_id,
                    std::optional<std::uint64_t> sent_at_ms);

  std::unique_ptr<tempolink::juceapp::network::IWebSocketTransport> transport_;
  std::mutex callback_mutex_;
  EventCallback callback_;
  std::string room_code_;
  std::string user_id_;
};
