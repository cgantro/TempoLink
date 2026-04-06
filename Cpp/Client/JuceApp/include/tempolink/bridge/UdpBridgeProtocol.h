#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace tempolink::bridge {

inline constexpr std::uint32_t kUdpBridgeMagic = 0x544C4247U;  // TLBG
inline constexpr std::uint16_t kUdpBridgeVersion = 1U;
inline constexpr char kDefaultBridgeHost[] = "127.0.0.1";
inline constexpr std::uint16_t kDefaultClientListenPort = 49010U;
inline constexpr std::uint16_t kDefaultPluginListenPort = 49011U;

enum class UdpBridgePacketType : std::uint16_t {
  kDawToClient = 1,
  kClientToDaw = 2,
};

struct UdpBridgePacketHeader {
  std::uint32_t magic = kUdpBridgeMagic;
  std::uint16_t version = kUdpBridgeVersion;
  UdpBridgePacketType type = UdpBridgePacketType::kDawToClient;
  std::uint32_t sample_rate_hz = 48000U;
  std::uint16_t channels = 2U;
  std::uint16_t frames = 0U;
  std::uint64_t sequence = 0U;
};

struct ParsedUdpBridgePacket {
  UdpBridgePacketHeader header{};
  std::vector<std::int16_t> samples;
};

std::vector<std::byte> SerializeUdpBridgePacket(
    const UdpBridgePacketHeader& header,
    std::span<const std::int16_t> samples);

bool ParseUdpBridgePacket(std::span<const std::byte> bytes,
                          ParsedUdpBridgePacket& packet);

}  // namespace tempolink::bridge

