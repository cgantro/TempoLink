#pragma once

#include <cstdint>
#include <functional>
#include <string>

#include <juce_core/juce_core.h>

namespace tempolink::juceapp::network {

/// Abstract WebSocket transport. Can be backed by JUCE StreamingSocket
/// (current impl), Boost.Beast, WebSocket++, or any other library.
class IWebSocketTransport {
 public:
  enum class Opcode : std::uint8_t {
    Text = 0x1,
    Binary = 0x2,
    Close = 0x8,
    Ping = 0x9,
    Pong = 0xA,
  };

  struct Frame {
    Opcode opcode;
    juce::MemoryBlock payload;
  };

  using OnFrameReceived = std::function<void(const Frame& frame)>;
  using OnDisconnected = std::function<void()>;

  virtual ~IWebSocketTransport() = default;

  /// Open a WebSocket connection (performs handshake).
  virtual bool Connect(const std::string& host, int port,
                       const std::string& path) = 0;

  /// Close the WebSocket connection.
  virtual void Disconnect() = 0;

  /// Check if the transport is connected.
  virtual bool IsConnected() const = 0;

  /// Send a framed message.
  virtual bool SendFrame(Opcode opcode, const juce::MemoryBlock& payload) = 0;

  /// Set the callback for incoming frames.
  virtual void SetOnFrameReceived(OnFrameReceived callback) = 0;

  /// Set the callback for disconnection.
  virtual void SetOnDisconnected(OnDisconnected callback) = 0;
};

}  // namespace tempolink::juceapp::network
