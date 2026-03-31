#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace tempolink::net {

enum class PacketType : std::uint8_t {
  kJoin = 1,
  kLeave = 2,
  kAudio = 3,
  kPing = 4,
  kPong = 5,
  kHeartbeat = 6,
  kControl = 7,
  kClockSync = 8,
  kClockSyncAck = 9,
  kStats = 10
};

struct PacketHeader {
  static constexpr std::uint32_t kMagic = 0x544C4B31;  // TLK1
  static constexpr std::uint8_t kVersion = 1;
  static constexpr std::size_t kEncodedSize = 40;

  std::uint32_t magic = kMagic;
  std::uint8_t version = kVersion;
  PacketType type = PacketType::kControl;
  std::uint16_t flags = 0;
  std::uint32_t room_id = 0;
  std::uint32_t sender_id = 0;
  std::uint32_t sequence = 0;
  std::uint64_t timestamp_us = 0;
  std::uint16_t stream_id = 0;
  std::uint16_t sample_rate_hz = 0;
  std::uint16_t frame_samples = 0;
  std::uint8_t channels = 0;
  std::uint8_t reserved = 0;
  std::uint32_t payload_size = 0;
};

struct Packet {
  PacketHeader header;
  std::vector<std::byte> payload;
};

}  // namespace tempolink::net
