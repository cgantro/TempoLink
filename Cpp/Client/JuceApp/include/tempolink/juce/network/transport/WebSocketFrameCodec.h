#pragma once

#include <cstdint>

#include <juce_core/juce_core.h>

#include "tempolink/juce/network/transport/IWebSocketTransport.h"

namespace tempolink::juceapp::network {

class SocketStream;

enum class WebSocketReadStatus {
  kOk,
  kTimeout,
  kClosed,
  kError,
  kProtocolError,
};

struct WebSocketReadResult {
  WebSocketReadStatus status = WebSocketReadStatus::kError;
  bool is_ping = false;
  tempolink::juceapp::network::IWebSocketTransport::Frame frame;
};

class WebSocketFrameCodec final {
 public:
  static bool WriteClientFrame(
      SocketStream& stream,
      tempolink::juceapp::network::IWebSocketTransport::Opcode opcode,
      const juce::MemoryBlock& payload);

  static WebSocketReadResult ReadServerFrame(SocketStream& stream,
                                             juce::MemoryBlock& pending_buffer,
                                             int timeout_ms);
};

}  // namespace tempolink::juceapp::network

