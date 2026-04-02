#pragma once

#include <optional>
#include <span>
#include <vector>

#include "tempolink/net/Packet.h"

namespace tempolink::net {

std::vector<std::byte> EncodePacket(const Packet& packet);
std::size_t EncodePacket(const Packet& packet, std::span<std::byte> destination);
std::optional<Packet> DecodePacket(std::span<const std::byte> bytes);

}  // namespace tempolink::net

