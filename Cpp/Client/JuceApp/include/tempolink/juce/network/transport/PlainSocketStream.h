#pragma once

#include "tempolink/juce/network/transport/ISocketStreamChannel.h"

namespace tempolink::juceapp::network {

class PlainSocketStream final : public ISocketStreamChannel {
 public:
  explicit PlainSocketStream(juce::StreamingSocket& socket);

  bool Initialize(const std::string& host, juce::String* error_text) override;
  void Shutdown() override;
  int ReadSome(void* out, int bytes, int timeout_ms) override;
  int WriteSome(const void* in, int bytes, int timeout_ms) override;

 private:
  juce::StreamingSocket& socket_;
};

}  // namespace tempolink::juceapp::network

