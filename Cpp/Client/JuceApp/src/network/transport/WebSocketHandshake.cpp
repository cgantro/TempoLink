#include "tempolink/juce/network/transport/WebSocketHandshake.h"

#include <string>

#include "tempolink/juce/network/transport/SocketStream.h"

namespace tempolink::juceapp::network {

namespace {

int FindHeaderEnd(const char* data, int size) {
  if (data == nullptr || size < 4) return -1;
  for (int i = 0; i <= size - 4; ++i) {
    if (data[i] == '\r' && data[i + 1] == '\n' &&
        data[i + 2] == '\r' && data[i + 3] == '\n') {
      return i + 4;
    }
  }
  return -1;
}

juce::String ExtractStatusLine(const std::string& response_header) {
  const auto pos = response_header.find("\r\n");
  if (pos == std::string::npos) {
    return juce::String(response_header);
  }
  return juce::String(response_header.substr(0, pos));
}

}  // namespace

WebSocketHandshakeResult WebSocketHandshake::Perform(SocketStream& stream,
                                                     const std::string& host,
                                                     int port,
                                                     const std::string& path) {
  WebSocketHandshakeResult result;

  std::array<std::uint8_t, 16> key_bytes{};
  for (auto& b : key_bytes) {
    b = static_cast<std::uint8_t>(juce::Random::getSystemRandom().nextInt(256));
  }
  const juce::String sec_key =
      juce::Base64::toBase64(key_bytes.data(), static_cast<int>(key_bytes.size()));

  juce::String request;
  request << "GET " << path << " HTTP/1.1\r\n";
  request << "Host: " << host << ":" << port << "\r\n";
  request << "Upgrade: websocket\r\n";
  request << "Connection: Upgrade\r\n";
  request << "Sec-WebSocket-Version: 13\r\n";
  request << "Sec-WebSocket-Key: " << sec_key << "\r\n\r\n";

  if (!stream.WriteAll(request.toRawUTF8(), request.getNumBytesAsUTF8(), 3000)) {
    result.error_text = "websocket handshake write failed";
    return result;
  }

  std::string response;
  response.reserve(2048);

  int elapsed_ms = 0;
  while (elapsed_ms < 2500) {
    char buffer[512];
    const int read = stream.ReadSome(buffer, static_cast<int>(sizeof(buffer)), 100);
    if (read == 0) {
      elapsed_ms += 100;
      continue;
    }
    if (read < 0) {
      result.error_text = "websocket handshake read failed";
      return result;
    }

    response.append(buffer, static_cast<std::size_t>(read));
    const int header_end = FindHeaderEnd(response.data(), static_cast<int>(response.size()));
    if (header_end <= 0) {
      continue;
    }

    const juce::String header =
        juce::String::fromUTF8(response.data(), header_end).trimEnd();
    result.status_line = ExtractStatusLine(response);
    if (!header.startsWithIgnoreCase("HTTP/1.1 101")) {
      result.error_text = "websocket upgrade rejected";
      return result;
    }

    const auto total_size = static_cast<int>(response.size());
    if (total_size > header_end) {
      result.pending_bytes.append(response.data() + header_end,
                                  static_cast<size_t>(total_size - header_end));
    }
    result.ok = true;
    return result;
  }

  result.error_text = "websocket handshake timed out";
  return result;
}

}  // namespace tempolink::juceapp::network

