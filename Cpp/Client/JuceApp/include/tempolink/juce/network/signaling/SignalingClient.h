#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <optional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <juce_core/juce_core.h>

class SignalingClient {
 public:
  struct Event {
    enum class Type { RoomJoined, PeerJoined, PeerLeft, PeerPing, PeerPong, Error };
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

  SignalingClient();
  ~SignalingClient();

  bool connect(const std::string& host, int port, const std::string& room_code,
               const std::string& user_id, EventCallback callback);
  void disconnect();
  bool isConnected() const;
  bool sendPeerPing(const std::string& to_user_id, std::uint64_t sent_at_ms);
  bool sendPeerPong(const std::string& to_user_id, std::uint64_t sent_at_ms);

 private:
  bool performHandshake(const std::string& host, int port, const std::string& path_query);
  void receiveLoop();
  bool readExact(void* out, int bytes, int timeout_ms);
  void dispatchTextMessage(const juce::String& message_text);
  void emitEvent(Event event);
  bool sendFrame(std::uint8_t opcode, const juce::MemoryBlock& payload);
  bool sendPong(const juce::MemoryBlock& payload);
  bool sendEnvelope(const juce::String& type,
                    const std::string& to_user_id,
                    std::optional<std::uint64_t> sent_at_ms);

  juce::StreamingSocket socket_;
  std::thread receive_thread_;
  std::atomic_bool connected_{false};
  std::mutex write_mutex_;
  std::mutex callback_mutex_;
  juce::MemoryBlock pending_read_buffer_;
  EventCallback callback_;
  std::string room_code_;
  std::string user_id_;
};
