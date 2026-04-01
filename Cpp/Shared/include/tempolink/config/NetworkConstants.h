#pragma once

#include <cstdint>

namespace tempolink::config {

inline constexpr char kDefaultRelayHost[] = "127.0.0.1";
inline constexpr std::uint16_t kDefaultRelayPort = 40000;

inline constexpr char kDefaultControlPlaneHost[] = "127.0.0.1";
inline constexpr std::uint16_t kDefaultControlPlanePort = 8080;
inline constexpr char kDefaultControlPlaneBaseUrl[] = "http://127.0.0.1:8080";

inline constexpr std::uint32_t kDefaultRoomId = 1;
inline constexpr std::uint32_t kDefaultParticipantId = 1001;
inline constexpr char kDefaultNickname[] = "guest";
inline constexpr char kDefaultEndpointProfileName[] = "default";

}  // namespace tempolink::config
