#include "tempolink/net/PacketCodec.h"

#include <cassert>
#include <cstring>

namespace tempolink::net {
namespace {

void WriteU16(std::byte* out, std::uint16_t value) {
  out[0] = static_cast<std::byte>((value >> 8) & 0xFF);
  out[1] = static_cast<std::byte>(value & 0xFF);
}

void WriteU32(std::byte* out, std::uint32_t value) {
  out[0] = static_cast<std::byte>((value >> 24) & 0xFF);
  out[1] = static_cast<std::byte>((value >> 16) & 0xFF);
  out[2] = static_cast<std::byte>((value >> 8) & 0xFF);
  out[3] = static_cast<std::byte>(value & 0xFF);
}

void WriteU64(std::byte* out, std::uint64_t value) {
  out[0] = static_cast<std::byte>((value >> 56) & 0xFF);
  out[1] = static_cast<std::byte>((value >> 48) & 0xFF);
  out[2] = static_cast<std::byte>((value >> 40) & 0xFF);
  out[3] = static_cast<std::byte>((value >> 32) & 0xFF);
  out[4] = static_cast<std::byte>((value >> 24) & 0xFF);
  out[5] = static_cast<std::byte>((value >> 16) & 0xFF);
  out[6] = static_cast<std::byte>((value >> 8) & 0xFF);
  out[7] = static_cast<std::byte>(value & 0xFF);
}

std::uint16_t ReadU16(const std::byte* in) {
  return (static_cast<std::uint16_t>(std::to_integer<unsigned int>(in[0])) << 8) |
         static_cast<std::uint16_t>(std::to_integer<unsigned int>(in[1]));
}

std::uint32_t ReadU32(const std::byte* in) {
  return (static_cast<std::uint32_t>(std::to_integer<unsigned int>(in[0])) << 24) |
         (static_cast<std::uint32_t>(std::to_integer<unsigned int>(in[1])) << 16) |
         (static_cast<std::uint32_t>(std::to_integer<unsigned int>(in[2])) << 8) |
         static_cast<std::uint32_t>(std::to_integer<unsigned int>(in[3]));
}

std::uint64_t ReadU64(const std::byte* in) {
  return (static_cast<std::uint64_t>(std::to_integer<unsigned int>(in[0])) << 56) |
         (static_cast<std::uint64_t>(std::to_integer<unsigned int>(in[1])) << 48) |
         (static_cast<std::uint64_t>(std::to_integer<unsigned int>(in[2])) << 40) |
         (static_cast<std::uint64_t>(std::to_integer<unsigned int>(in[3])) << 32) |
         (static_cast<std::uint64_t>(std::to_integer<unsigned int>(in[4])) << 24) |
         (static_cast<std::uint64_t>(std::to_integer<unsigned int>(in[5])) << 16) |
         (static_cast<std::uint64_t>(std::to_integer<unsigned int>(in[6])) << 8) |
         static_cast<std::uint64_t>(std::to_integer<unsigned int>(in[7]));
}

}  // namespace

std::vector<std::byte> EncodePacket(const Packet& packet) {
  PacketHeader header = packet.header;
  header.payload_size = static_cast<std::uint32_t>(packet.payload.size());

  std::vector<std::byte> encoded(PacketHeader::kEncodedSize +
                                 packet.payload.size());
  std::byte* out = encoded.data();
  WriteU32(out, header.magic);
  out[4] = static_cast<std::byte>(header.version);
  out[5] = static_cast<std::byte>(header.type);
  WriteU16(out + 6, header.flags);
  WriteU32(out + 8, header.room_id);
  WriteU32(out + 12, header.sender_id);
  WriteU32(out + 16, header.sequence);
  WriteU64(out + 20, header.timestamp_us);
  WriteU16(out + 28, header.stream_id);
  WriteU16(out + 30, header.sample_rate_hz);
  WriteU16(out + 32, header.frame_samples);
  out[34] = static_cast<std::byte>(header.channels);
  out[35] = static_cast<std::byte>(header.reserved);
  WriteU32(out + 36, header.payload_size);

  if (!packet.payload.empty()) {
    assert(out != nullptr);
    assert(packet.payload.data() != nullptr);
    assert(packet.payload.size() <= encoded.size() - PacketHeader::kEncodedSize);
    std::memcpy(out + PacketHeader::kEncodedSize, packet.payload.data(),
                packet.payload.size());
  }

  return encoded;
}

std::optional<Packet> DecodePacket(std::span<const std::byte> bytes) {
  if (bytes.size() < PacketHeader::kEncodedSize) {
    return std::nullopt;
  }

  const std::byte* in = bytes.data();
  Packet packet;
  packet.header.magic = ReadU32(in);
  packet.header.version = static_cast<std::uint8_t>(in[4]);
  packet.header.type = static_cast<PacketType>(in[5]);
  packet.header.flags = ReadU16(in + 6);
  packet.header.room_id = ReadU32(in + 8);
  packet.header.sender_id = ReadU32(in + 12);
  packet.header.sequence = ReadU32(in + 16);
  packet.header.timestamp_us = ReadU64(in + 20);
  packet.header.stream_id = ReadU16(in + 28);
  packet.header.sample_rate_hz = ReadU16(in + 30);
  packet.header.frame_samples = ReadU16(in + 32);
  packet.header.channels = static_cast<std::uint8_t>(in[34]);
  packet.header.reserved = static_cast<std::uint8_t>(in[35]);
  packet.header.payload_size = ReadU32(in + 36);

  if (packet.header.magic != PacketHeader::kMagic ||
      packet.header.version != PacketHeader::kVersion) {
    return std::nullopt;
  }

  if (PacketHeader::kEncodedSize + packet.header.payload_size > bytes.size()) {
    return std::nullopt;
  }

  packet.payload.resize(packet.header.payload_size);
  if (packet.header.payload_size > 0) {
    assert(packet.payload.data() != nullptr);
    assert(in != nullptr);
    assert(packet.header.payload_size <= bytes.size() - PacketHeader::kEncodedSize);
    std::memcpy(packet.payload.data(), in + PacketHeader::kEncodedSize,
                packet.header.payload_size);
  }

  return packet;
}

}  // namespace tempolink::net
