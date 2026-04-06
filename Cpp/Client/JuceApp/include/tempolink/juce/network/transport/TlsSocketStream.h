#pragma once

#include "tempolink/juce/network/transport/ISocketStreamChannel.h"

namespace tempolink::juceapp::network {

class TlsSocketStream final : public ISocketStreamChannel {
 public:
  explicit TlsSocketStream(juce::StreamingSocket& socket);
  ~TlsSocketStream() override;

  bool Initialize(const std::string& host, juce::String* error_text) override;
  void Shutdown() override;
  int ReadSome(void* out, int bytes, int timeout_ms) override;
  int WriteSome(const void* in, int bytes, int timeout_ms) override;

 private:
  juce::StreamingSocket& socket_;
  void* ssl_ctx_ = nullptr;
  void* ssl_ = nullptr;
};

}  // namespace tempolink::juceapp::network

