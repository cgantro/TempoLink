#pragma once

#include <memory>
#include <string>

#include <juce_core/juce_core.h>

#include "tempolink/juce/network/transport/ISocketStreamChannel.h"

namespace tempolink::juceapp::network {

/// Stream abstraction over plain TCP and TLS(TCP+OpenSSL).
class SocketStream final {
 public:
  SocketStream() = default;
  ~SocketStream();

  bool Connect(const std::string& host, int port, bool use_tls, int timeout_ms,
               juce::String* error_text = nullptr);
  void Close();

  int ReadSome(void* out, int bytes, int timeout_ms);
  int WriteSome(const void* in, int bytes, int timeout_ms);
  bool WriteAll(const void* in, int bytes, int timeout_ms);

 private:
  juce::StreamingSocket socket_;
  std::unique_ptr<ISocketStreamChannel> channel_;
};

}  // namespace tempolink::juceapp::network
