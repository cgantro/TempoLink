#pragma once

#include <string>

#include <juce_core/juce_core.h>

namespace tempolink::juceapp::network {

class SocketStream;

struct WebSocketHandshakeResult {
  bool ok = false;
  juce::String error_text;
  juce::String status_line;
  juce::MemoryBlock pending_bytes;
};

class WebSocketHandshake final {
 public:
  static WebSocketHandshakeResult Perform(SocketStream& stream,
                                          const std::string& host, int port,
                                          const std::string& path);
};

}  // namespace tempolink::juceapp::network

