#pragma once

#include "tempolink/juce/network/transport/IWebSocketTransport.h"
#include "tempolink/juce/network/transport/JuceWebSocketTransport.h"

namespace tempolink::juceapp::network {

class PlainWebSocketTransport final : public IWebSocketTransport {
 public:
  bool Connect(const std::string& host, int port,
               const std::string& path, bool use_tls = false) override;
  void Disconnect() override;
  bool IsConnected() const override;
  bool SendFrame(Opcode opcode, const juce::MemoryBlock& payload) override;
  void SetOnFrameReceived(OnFrameReceived callback) override;
  void SetOnDisconnected(OnDisconnected callback) override;

 private:
  JuceWebSocketTransport inner_;
};

}  // namespace tempolink::juceapp::network

