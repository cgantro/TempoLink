#include "tempolink/juce/network/transport/SocketStream.h"

#include "tempolink/juce/network/transport/PlainSocketStream.h"
#include "tempolink/juce/network/transport/TlsSocketStream.h"

namespace tempolink::juceapp::network {

SocketStream::~SocketStream() { Close(); }

bool SocketStream::Connect(const std::string& host, int port, bool use_tls,
                           int timeout_ms, juce::String* error_text) {
  Close();

  if (!socket_.connect(host, port, timeout_ms)) {
    if (error_text != nullptr) {
      *error_text = "tcp connect failed";
    }
    return false;
  }

  if (use_tls) {
    channel_ = std::make_unique<TlsSocketStream>(socket_);
  } else {
    channel_ = std::make_unique<PlainSocketStream>(socket_);
  }

  if (!channel_->Initialize(host, error_text)) {
    Close();
    return false;
  }

  return true;
}

void SocketStream::Close() {
  if (channel_) {
    channel_->Shutdown();
    channel_.reset();
  }
  socket_.close();
}

int SocketStream::ReadSome(void* out, int bytes, int timeout_ms) {
  if (!channel_) {
    return -1;
  }
  return channel_->ReadSome(out, bytes, timeout_ms);
}

int SocketStream::WriteSome(const void* in, int bytes, int timeout_ms) {
  if (!channel_) {
    return -1;
  }
  return channel_->WriteSome(in, bytes, timeout_ms);
}

bool SocketStream::WriteAll(const void* in, int bytes, int timeout_ms) {
  if (bytes <= 0 || in == nullptr) {
    return bytes == 0;
  }
  const auto* src = static_cast<const std::uint8_t*>(in);
  int written = 0;
  while (written < bytes) {
    const int n = WriteSome(src + written, bytes - written, timeout_ms);
    if (n <= 0) {
      return false;
    }
    written += n;
  }
  return true;
}

}  // namespace tempolink::juceapp::network

