#include "tempolink/juce/network/transport/JuceWebSocketTransport.h"

#include <algorithm>
#include <cstring>
#include <limits>
#include <utility>

#include "tempolink/juce/logging/AppLogger.h"

namespace tempolink::juceapp::network {

namespace {

std::uint16_t ReadU16BE(const std::uint8_t* bytes) {
  return static_cast<std::uint16_t>((bytes[0] << 8) | bytes[1]);
}

std::uint64_t ReadU64BE(const std::uint8_t* bytes) {
  std::uint64_t value = 0;
  for (int i = 0; i < 8; ++i) value = (value << 8) | bytes[i];
  return value;
}

int FindHeaderEnd(const char* data, int size) {
  if (data == nullptr || size < 4) return -1;
  for (int i = 0; i <= size - 4; ++i) {
    if (data[i] == '\r' && data[i + 1] == '\n' &&
        data[i + 2] == '\r' && data[i + 3] == '\n')
      return i + 4;
  }
  return -1;
}

constexpr std::uint64_t kMaxPayloadBytes = 1024 * 1024;

}  // namespace

JuceWebSocketTransport::JuceWebSocketTransport() = default;

JuceWebSocketTransport::~JuceWebSocketTransport() { Disconnect(); }

bool JuceWebSocketTransport::Connect(const std::string& host, int port,
                                     const std::string& path) {
  Disconnect();
  if (!PerformHandshake(host, port, path)) return false;
  connected_ = true;
  receive_thread_ = std::thread([this] { ReceiveLoop(); });
  return true;
}

void JuceWebSocketTransport::Disconnect() {
  const bool was = connected_.load();
  if (was) {
    const juce::MemoryBlock empty;
    SendFrame(Opcode::Close, empty);
  }
  connected_ = false;
  socket_.close();
  if (receive_thread_.joinable()) receive_thread_.join();
}

bool JuceWebSocketTransport::IsConnected() const { return connected_.load(); }

bool JuceWebSocketTransport::SendFrame(Opcode opcode,
                                       const juce::MemoryBlock& payload) {
  if (!connected_) return false;
  std::lock_guard<std::mutex> lock(write_mutex_);

  juce::MemoryOutputStream frame;
  const std::uint8_t fin_opcode =
      0x80U | (static_cast<std::uint8_t>(opcode) & 0x0FU);
  frame.writeByte(static_cast<char>(fin_opcode));

  const std::size_t payload_size = payload.getSize();
  if (payload_size <= 125) {
    frame.writeByte(
        static_cast<char>(0x80U | static_cast<std::uint8_t>(payload_size)));
  } else if (payload_size <= 65535) {
    frame.writeByte(static_cast<char>(0x80U | 126U));
    const auto len = static_cast<std::uint16_t>(payload_size);
    frame.writeByte(static_cast<char>((len >> 8) & 0xFF));
    frame.writeByte(static_cast<char>(len & 0xFF));
  } else {
    frame.writeByte(static_cast<char>(0x80U | 127U));
    const auto len = static_cast<std::uint64_t>(payload_size);
    for (int i = 7; i >= 0; --i)
      frame.writeByte(static_cast<char>((len >> (i * 8)) & 0xFF));
  }

  std::array<std::uint8_t, 4> mask{};
  juce::Random::getSystemRandom().fillBitsRandomly(mask.data(), mask.size());
  frame.write(mask.data(), mask.size());

  const auto* raw = static_cast<const std::uint8_t*>(payload.getData());
  for (std::size_t i = 0; i < payload_size; ++i)
    frame.writeByte(static_cast<char>(raw[i] ^ mask[i % 4]));

  return socket_.write(frame.getData(),
                       static_cast<int>(frame.getDataSize())) > 0;
}

void JuceWebSocketTransport::SetOnFrameReceived(OnFrameReceived callback) {
  std::lock_guard<std::mutex> lock(callback_mutex_);
  on_frame_received_ = std::move(callback);
}

void JuceWebSocketTransport::SetOnDisconnected(OnDisconnected callback) {
  std::lock_guard<std::mutex> lock(callback_mutex_);
  on_disconnected_ = std::move(callback);
}

bool JuceWebSocketTransport::PerformHandshake(const std::string& host,
                                               int port,
                                               const std::string& path) {
  pending_read_buffer_.reset();
  if (!socket_.connect(host, port, 3000)) return false;

  std::array<std::uint8_t, 16> key_bytes{};
  for (auto& b : key_bytes)
    b = static_cast<std::uint8_t>(juce::Random::getSystemRandom().nextInt(256));
  const juce::String sec_key =
      juce::Base64::toBase64(key_bytes.data(),
                             static_cast<int>(key_bytes.size()));

  juce::String request;
  request << "GET " << path << " HTTP/1.1\r\n";
  request << "Host: " << host << ":" << port << "\r\n";
  request << "Upgrade: websocket\r\n";
  request << "Connection: Upgrade\r\n";
  request << "Sec-WebSocket-Version: 13\r\n";
  request << "Sec-WebSocket-Key: " << sec_key << "\r\n\r\n";

  if (socket_.write(request.toRawUTF8(), request.getNumBytesAsUTF8()) <= 0) {
    socket_.close();
    return false;
  }

  std::string response;
  response.reserve(2048);
  int elapsed = 0;
  while (elapsed < 2500) {
    if (socket_.waitUntilReady(true, 100) <= 0) {
      elapsed += 100;
      continue;
    }
    char buffer[512];
    const int read =
        socket_.read(buffer, static_cast<int>(sizeof(buffer)), false);
    if (read <= 0) break;
    response.append(buffer, static_cast<std::size_t>(read));
    const int header_end =
        FindHeaderEnd(response.data(), static_cast<int>(response.size()));
    if (header_end > 0) {
      const juce::String header =
          juce::String::fromUTF8(response.data(), header_end).trimEnd();
      if (!header.startsWithIgnoreCase("HTTP/1.1 101")) break;
      const auto size = static_cast<int>(response.size());
      if (size > header_end) {
        pending_read_buffer_.append(response.data() + header_end,
                                    static_cast<size_t>(size - header_end));
      }
      return true;
    }
  }
  socket_.close();
  return false;
}

void JuceWebSocketTransport::ReceiveLoop() {
  while (connected_) {
    std::uint8_t header[2];
    if (!ReadExact(header, 2, 1000)) continue;

    const std::uint8_t opcode = header[0] & 0x0F;
    const bool masked = (header[1] & 0x80U) != 0;
    std::uint64_t payload_len =
        static_cast<std::uint64_t>(header[1] & 0x7FU);

    if (payload_len == 126) {
      std::uint8_t ext[2];
      if (!ReadExact(ext, 2, 1000)) break;
      payload_len = ReadU16BE(ext);
    } else if (payload_len == 127) {
      std::uint8_t ext[8];
      if (!ReadExact(ext, 8, 1000)) break;
      payload_len = ReadU64BE(ext);
    }

    std::array<std::uint8_t, 4> mask_key{0, 0, 0, 0};
    if (masked && !ReadExact(mask_key.data(), 4, 1000)) break;

    if (payload_len > kMaxPayloadBytes ||
        payload_len >
            static_cast<std::uint64_t>(std::numeric_limits<int>::max()))
      break;

    juce::MemoryBlock payload;
    payload.setSize(static_cast<size_t>(payload_len));
    if (payload_len > 0 &&
        !ReadExact(payload.getData(), static_cast<int>(payload_len), 1000))
      break;

    if (masked) {
      auto* bytes = static_cast<std::uint8_t*>(payload.getData());
      for (std::uint64_t i = 0; i < payload_len; ++i)
        bytes[i] ^= mask_key[static_cast<std::size_t>(i % 4)];
    }

    if (opcode == 0x8) break;  // close
    if (opcode == 0x9) {       // ping → pong
      SendFrame(Opcode::Pong, payload);
      continue;
    }

    Frame frame;
    frame.opcode = static_cast<Opcode>(opcode);
    frame.payload = std::move(payload);

    OnFrameReceived callback;
    {
      std::lock_guard<std::mutex> lock(callback_mutex_);
      callback = on_frame_received_;
    }
    if (callback) callback(frame);
  }

  connected_ = false;

  OnDisconnected dc_callback;
  {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    dc_callback = on_disconnected_;
  }
  if (dc_callback) dc_callback();
}

bool JuceWebSocketTransport::ReadExact(void* out, int bytes, int timeout_ms) {
  if (out == nullptr || bytes <= 0) return bytes == 0;
  auto* dst = static_cast<char*>(out);
  int total = 0;

  if (pending_read_buffer_.getSize() > 0) {
    const int cached =
        std::min(bytes, static_cast<int>(pending_read_buffer_.getSize()));
    std::memcpy(dst, pending_read_buffer_.getData(),
                static_cast<size_t>(cached));
    pending_read_buffer_.removeSection(0, static_cast<size_t>(cached));
    total += cached;
  }

  while (total < bytes) {
    if (!connected_) return false;
    if (socket_.waitUntilReady(true, timeout_ms) <= 0) return false;
    const int n = socket_.read(dst + total, bytes - total, false);
    if (n <= 0) return false;
    total += n;
  }
  return true;
}

}  // namespace tempolink::juceapp::network
