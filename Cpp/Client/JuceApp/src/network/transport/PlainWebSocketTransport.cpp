#include "tempolink/juce/network/transport/PlainWebSocketTransport.h"

#include <utility>

namespace tempolink::juceapp::network {

bool PlainWebSocketTransport::Connect(const std::string& host, int port,
                                      const std::string& path, bool use_tls) {
  juce::ignoreUnused(use_tls);
  return inner_.Connect(host, port, path, false);
}

void PlainWebSocketTransport::Disconnect() { inner_.Disconnect(); }

bool PlainWebSocketTransport::IsConnected() const { return inner_.IsConnected(); }

bool PlainWebSocketTransport::SendFrame(Opcode opcode,
                                        const juce::MemoryBlock& payload) {
  return inner_.SendFrame(opcode, payload);
}

void PlainWebSocketTransport::SetOnFrameReceived(OnFrameReceived callback) {
  inner_.SetOnFrameReceived(std::move(callback));
}

void PlainWebSocketTransport::SetOnDisconnected(OnDisconnected callback) {
  inner_.SetOnDisconnected(std::move(callback));
}

}  // namespace tempolink::juceapp::network
