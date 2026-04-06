#include "tempolink/juce/network/transport/PlainSocketStream.h"

namespace tempolink::juceapp::network {

PlainSocketStream::PlainSocketStream(juce::StreamingSocket& socket)
    : socket_(socket) {}

bool PlainSocketStream::Initialize(const std::string& host,
                                   juce::String* error_text) {
  juce::ignoreUnused(host, error_text);
  return true;
}

void PlainSocketStream::Shutdown() {}

int PlainSocketStream::ReadSome(void* out, int bytes, int timeout_ms) {
  if (bytes <= 0 || out == nullptr) {
    return 0;
  }
  if (socket_.waitUntilReady(true, timeout_ms) <= 0) {
    return 0;
  }
  return socket_.read(out, bytes, false);
}

int PlainSocketStream::WriteSome(const void* in, int bytes, int timeout_ms) {
  if (bytes <= 0 || in == nullptr) {
    return 0;
  }
  if (socket_.waitUntilReady(false, timeout_ms) <= 0) {
    return 0;
  }
  return socket_.write(in, bytes);
}

}  // namespace tempolink::juceapp::network

