#pragma once

#include <array>
#include <atomic>
#include <mutex>
#include <thread>

#include <juce_core/juce_core.h>

#include "tempolink/juce/network/transport/IWebSocketTransport.h"

namespace tempolink::juceapp::network {

/// JUCE StreamingSocket-based WebSocket transport.
/// Extracted from the old monolithic SignalingClient.
class JuceWebSocketTransport final : public IWebSocketTransport {
 public:
  JuceWebSocketTransport();
  ~JuceWebSocketTransport() override;

  bool Connect(const std::string& host, int port,
               const std::string& path) override;
  void Disconnect() override;
  bool IsConnected() const override;
  bool SendFrame(Opcode opcode, const juce::MemoryBlock& payload) override;
  void SetOnFrameReceived(OnFrameReceived callback) override;
  void SetOnDisconnected(OnDisconnected callback) override;

 private:
  bool PerformHandshake(const std::string& host, int port,
                        const std::string& path);
  void ReceiveLoop();
  bool ReadExact(void* out, int bytes, int timeout_ms);

  juce::StreamingSocket socket_;
  std::thread receive_thread_;
  std::atomic_bool connected_{false};
  std::mutex write_mutex_;
  std::mutex callback_mutex_;
  juce::MemoryBlock pending_read_buffer_;
  OnFrameReceived on_frame_received_;
  OnDisconnected on_disconnected_;
};

}  // namespace tempolink::juceapp::network
