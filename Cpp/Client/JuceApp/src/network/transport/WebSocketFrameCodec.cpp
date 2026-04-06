#include "tempolink/juce/network/transport/WebSocketFrameCodec.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <limits>

#include "tempolink/juce/network/transport/SocketStream.h"

namespace tempolink::juceapp::network {

namespace {

constexpr std::uint64_t kMaxPayloadBytes = 1024 * 1024;

std::uint16_t ReadU16BE(const std::uint8_t* bytes) {
  return static_cast<std::uint16_t>((bytes[0] << 8) | bytes[1]);
}

std::uint64_t ReadU64BE(const std::uint8_t* bytes) {
  std::uint64_t value = 0;
  for (int i = 0; i < 8; ++i) {
    value = (value << 8) | bytes[i];
  }
  return value;
}

WebSocketReadStatus ReadExact(SocketStream& stream, juce::MemoryBlock& pending,
                              void* out, int bytes, int timeout_ms) {
  if (out == nullptr || bytes <= 0) {
    return bytes == 0 ? WebSocketReadStatus::kOk : WebSocketReadStatus::kError;
  }

  auto* dst = static_cast<char*>(out);
  int total = 0;

  if (pending.getSize() > 0) {
    const int cached = (std::min)(bytes, static_cast<int>(pending.getSize()));
    std::memcpy(dst, pending.getData(), static_cast<size_t>(cached));
    pending.removeSection(0, static_cast<size_t>(cached));
    total += cached;
  }

  while (total < bytes) {
    const int n = stream.ReadSome(dst + total, bytes - total, timeout_ms);
    if (n == 0) {
      return WebSocketReadStatus::kTimeout;
    }
    if (n < 0) {
      return WebSocketReadStatus::kError;
    }
    total += n;
  }

  return WebSocketReadStatus::kOk;
}

}  // namespace

bool WebSocketFrameCodec::WriteClientFrame(
    SocketStream& stream, tempolink::juceapp::network::IWebSocketTransport::Opcode opcode,
    const juce::MemoryBlock& payload) {
  juce::MemoryOutputStream frame;
  const std::uint8_t fin_opcode =
      0x80U | (static_cast<std::uint8_t>(opcode) & 0x0FU);
  frame.writeByte(static_cast<char>(fin_opcode));

  const std::size_t payload_size = payload.getSize();
  if (payload_size <= 125) {
    frame.writeByte(static_cast<char>(0x80U | static_cast<std::uint8_t>(payload_size)));
  } else if (payload_size <= 65535) {
    frame.writeByte(static_cast<char>(0x80U | 126U));
    const auto len = static_cast<std::uint16_t>(payload_size);
    frame.writeByte(static_cast<char>((len >> 8) & 0xFF));
    frame.writeByte(static_cast<char>(len & 0xFF));
  } else {
    frame.writeByte(static_cast<char>(0x80U | 127U));
    const auto len = static_cast<std::uint64_t>(payload_size);
    for (int i = 7; i >= 0; --i) {
      frame.writeByte(static_cast<char>((len >> (i * 8)) & 0xFF));
    }
  }

  std::array<std::uint8_t, 4> mask{};
  juce::Random::getSystemRandom().fillBitsRandomly(mask.data(), mask.size());
  frame.write(mask.data(), mask.size());

  const auto* raw = static_cast<const std::uint8_t*>(payload.getData());
  for (std::size_t i = 0; i < payload_size; ++i) {
    frame.writeByte(static_cast<char>(raw[i] ^ mask[i % 4]));
  }

  return stream.WriteAll(frame.getData(), static_cast<int>(frame.getDataSize()), 3000);
}

WebSocketReadResult WebSocketFrameCodec::ReadServerFrame(SocketStream& stream,
                                                         juce::MemoryBlock& pending_buffer,
                                                         int timeout_ms) {
  WebSocketReadResult result;

  std::uint8_t header[2];
  const auto header_status =
      ReadExact(stream, pending_buffer, header, 2, timeout_ms);
  if (header_status != WebSocketReadStatus::kOk) {
    result.status = header_status;
    return result;
  }

  const std::uint8_t opcode = header[0] & 0x0F;
  const bool masked = (header[1] & 0x80U) != 0;
  std::uint64_t payload_len = static_cast<std::uint64_t>(header[1] & 0x7FU);

  if (payload_len == 126) {
    std::uint8_t ext[2];
    const auto ext_status = ReadExact(stream, pending_buffer, ext, 2, timeout_ms);
    if (ext_status != WebSocketReadStatus::kOk) {
      result.status = ext_status;
      return result;
    }
    payload_len = ReadU16BE(ext);
  } else if (payload_len == 127) {
    std::uint8_t ext[8];
    const auto ext_status = ReadExact(stream, pending_buffer, ext, 8, timeout_ms);
    if (ext_status != WebSocketReadStatus::kOk) {
      result.status = ext_status;
      return result;
    }
    payload_len = ReadU64BE(ext);
  }

  if (payload_len > kMaxPayloadBytes ||
      payload_len > static_cast<std::uint64_t>((std::numeric_limits<int>::max)())) {
    result.status = WebSocketReadStatus::kProtocolError;
    return result;
  }

  std::array<std::uint8_t, 4> mask_key{0, 0, 0, 0};
  if (masked) {
    const auto mask_status =
        ReadExact(stream, pending_buffer, mask_key.data(), 4, timeout_ms);
    if (mask_status != WebSocketReadStatus::kOk) {
      result.status = mask_status;
      return result;
    }
  }

  juce::MemoryBlock payload;
  payload.setSize(static_cast<size_t>(payload_len));
  if (payload_len > 0) {
    const auto payload_status =
        ReadExact(stream, pending_buffer, payload.getData(), static_cast<int>(payload_len),
                  timeout_ms);
    if (payload_status != WebSocketReadStatus::kOk) {
      result.status = payload_status;
      return result;
    }
  }

  if (masked) {
    auto* bytes = static_cast<std::uint8_t*>(payload.getData());
    for (std::uint64_t i = 0; i < payload_len; ++i) {
      bytes[i] ^= mask_key[static_cast<std::size_t>(i % 4)];
    }
  }

  if (opcode == 0x8U) {
    result.status = WebSocketReadStatus::kClosed;
    return result;
  }

  result.status = WebSocketReadStatus::kOk;
  result.is_ping = opcode == 0x9U;
  result.frame.opcode =
      static_cast<tempolink::juceapp::network::IWebSocketTransport::Opcode>(opcode);
  result.frame.payload = std::move(payload);
  return result;
}

}  // namespace tempolink::juceapp::network

