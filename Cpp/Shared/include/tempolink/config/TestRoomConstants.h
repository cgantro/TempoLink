#pragma once

#include <cstdint>

#include "tempolink/config/NetworkConstants.h"


/**
 * @brief 이것도 결국엔 서버에서 받아와야한다.
 */
namespace tempolink::config {

inline constexpr char kTestRoomCode[] = "TEST00";
inline constexpr char kTestRoomTitle[] = "TestRoom";
inline constexpr char kTestRoomHostUserId[] = "test-room-host";
inline constexpr char kTestRoomHostLabel[] = "System";
inline constexpr char kTestRoomGenre[] = "Network/Audio Test";
inline constexpr char kTestRoomLatencyHint[] = "Latency check";
inline constexpr char kTestRoomApiLatencyHint[] = "Audio/Network Test";
inline constexpr int kTestRoomMaxParticipants = 6;
inline constexpr std::uint16_t kTestRoomRelayPort = kDefaultRelayPort;

}  // namespace tempolink::config
