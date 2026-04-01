#include "tempolink/bridge/UdpBridgeProtocol.h"

#include <cstring>

namespace tempolink::bridge {
namespace {

constexpr std::size_t kHeaderBytes = 24U;

void WriteU16BE(std::byte* out, std::uint16_t value) {
  out[0] = static_cast<std::byte>((value >> 8) & 0xFFU);
  out[1] = static_cast<std::byte>(value & 0xFFU);
}

void WriteU32BE(std::byte* out, std::uint32_t value) {
  out[0] = static_cast<std::byte>((value >> 24) & 0xFFU);
  out[1] = static_cast<std::byte>((value >> 16) & 0xFFU);
  out[2] = static_cast<std::byte>((value >> 8) & 0xFFU);
  out[3] = static_cast<std::byte>(value & 0xFFU);
}

void WriteU64BE(std::byte* out, std::uint64_t value) {
  out[0] = static_cast<std::byte>((value >> 56) & 0xFFU);
  out[1] = static_cast<std::byte>((value >> 48) & 0xFFU);
  out[2] = static_cast<std::byte>((value >> 40) & 0xFFU);
  out[3] = static_cast<std::byte>((value >> 32) & 0xFFU);
  out[4] = static_cast<std::byte>((value >> 24) & 0xFFU);
  out[5] = static_cast<std::byte>((value >> 16) & 0xFFU);
  out[6] = static_cast<std::byte>((value >> 8) & 0xFFU);
  out[7] = static_cast<std::byte>(value & 0xFFU);
}

std::uint16_t ReadU16BE(const std::byte* in) {
  return (static_cast<std::uint16_t>(std::to_integer<unsigned int>(in[0])) << 8) |
         static_cast<std::uint16_t>(std::to_integer<unsigned int>(in[1]));
}

std::uint32_t ReadU32BE(const std::byte* in) {
  return (static_cast<std::uint32_t>(std::to_integer<unsigned int>(in[0])) << 24) |
         (static_cast<std::uint32_t>(std::to_integer<unsigned int>(in[1])) << 16) |
         (static_cast<std::uint32_t>(std::to_integer<unsigned int>(in[2])) << 8) |
         static_cast<std::uint32_t>(std::to_integer<unsigned int>(in[3]));
}

std::uint64_t ReadU64BE(const std::byte* in) {
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

std::vector<std::byte> SerializeUdpBridgePacket(
    const UdpBridgePacketHeader& header,
    std::span<const std::int16_t> samples) {
  std::vector<std::byte> packet(
      kHeaderBytes + samples.size() * sizeof(std::int16_t));

  WriteU32BE(packet.data() + 0, header.magic);
  WriteU16BE(packet.data() + 4, header.version);
  WriteU16BE(packet.data() + 6, static_cast<std::uint16_t>(header.type));
  WriteU32BE(packet.data() + 8, header.sample_rate_hz);
  WriteU16BE(packet.data() + 12, header.channels);
  WriteU16BE(packet.data() + 14, header.frames);
  WriteU64BE(packet.data() + 16, header.sequence);

  if (!samples.empty()) {
    std::memcpy(packet.data() + kHeaderBytes, samples.data(),
                samples.size() * sizeof(std::int16_t));
  }
  return packet;
}

bool ParseUdpBridgePacket(std::span<const std::byte> bytes,
                          ParsedUdpBridgePacket& packet) {
  if (bytes.size() < kHeaderBytes) {
    return false;
  }

  UdpBridgePacketHeader header;
  header.magic = ReadU32BE(bytes.data() + 0);
  header.version = ReadU16BE(bytes.data() + 4);
  header.type = static_cast<UdpBridgePacketType>(ReadU16BE(bytes.data() + 6));
  header.sample_rate_hz = ReadU32BE(bytes.data() + 8);
  header.channels = ReadU16BE(bytes.data() + 12);
  header.frames = ReadU16BE(bytes.data() + 14);
  header.sequence = ReadU64BE(bytes.data() + 16);

  if (header.magic != kUdpBridgeMagic || header.version != kUdpBridgeVersion ||
      header.channels == 0U || header.frames == 0U) {
    return false;
  }

  const std::size_t sample_count =
      static_cast<std::size_t>(header.channels) * static_cast<std::size_t>(header.frames);
  const std::size_t payload_bytes = sample_count * sizeof(std::int16_t);
  if (bytes.size() != kHeaderBytes + payload_bytes) {
    return false;
  }

  packet.header = header;
  packet.samples.resize(sample_count);
  if (payload_bytes > 0) {
    std::memcpy(packet.samples.data(), bytes.data() + kHeaderBytes, payload_bytes);
  }
  return true;
}

}  // namespace tempolink::bridge

