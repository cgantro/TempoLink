#include "tempolink/juce/network/transport/TlsWebSocketTransport.h"

#include <utility>

namespace tempolink::juceapp::network {

bool TlsWebSocketTransport::Connect(const std::string& host, int port,
                                    const std::string& path, bool use_tls) {
  juce::ignoreUnused(use_tls);
  return inner_.Connect(host, port, path, true);
}

void TlsWebSocketTransport::Disconnect() { inner_.Disconnect(); }

bool TlsWebSocketTransport::IsConnected() const { return inner_.IsConnected(); }

bool TlsWebSocketTransport::SendFrame(Opcode opcode,
                                      const juce::MemoryBlock& payload) {
  return inner_.SendFrame(opcode, payload);
}

void TlsWebSocketTransport::SetOnFrameReceived(OnFrameReceived callback) {
  inner_.SetOnFrameReceived(std::move(callback));
}

void TlsWebSocketTransport::SetOnDisconnected(OnDisconnected callback) {
  inner_.SetOnDisconnected(std::move(callback));
}

}  // namespace tempolink::juceapp::network
