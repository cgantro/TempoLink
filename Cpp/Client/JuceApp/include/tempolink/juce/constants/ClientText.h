#pragma once

namespace tempolink::juceapp::text {

inline constexpr char kLoadingRooms[] = "Loading rooms from control-plane...";
inline constexpr char kRoomApiUnavailablePrefix[] = "Room API unavailable: ";
inline constexpr char kShowingMockRoomsSuffix[] = " | room list cleared";
inline constexpr char kRoomApiEmptyFallback[] = "Room API returned empty list";
inline constexpr char kLoadedRoomsPrefix[] = "Loaded ";
inline constexpr char kLoadedRoomsUserDelimiter[] = " rooms. User: ";
inline constexpr char kIceConfigUnavailablePrefix[] = "ICE config unavailable: ";
inline constexpr char kNetworkProfileLoadedPrefix[] = "Network profile loaded (";
inline constexpr char kNetworkProfileLoadedSuffix[] = ")";
inline constexpr char kP2PAssistReady[] = "P2P assist ready";
inline constexpr char kP2PAssistLimited[] = "P2P assist limited";
inline constexpr char kTurnFallbackReady[] = "TURN fallback ready";
inline constexpr char kTurnFallbackUnavailable[] = "TURN fallback unavailable";

inline constexpr char kRoomNotFound[] = "Room not found.";
inline constexpr char kEntryPreflightGuide[] = "Check part and audio devices, then join.";
inline constexpr char kCreatingRoom[] = "Creating room...";
inline constexpr char kCreateRoomFailedPrefix[] = "Create room failed: ";
inline constexpr char kCreatedRoomPrefix[] = "Created room ";
inline constexpr char kCreatedRoomSuffix[] = ".";
inline constexpr char kJoiningRoomPrefix[] = "Joining room ";
inline constexpr char kJoinPendingSuffix[] = " ...";
inline constexpr char kJoinFailedPrefix[] = "Join failed: ";
inline constexpr char kJoinMetadataMissing[] = "Joined but room metadata missing.";
inline constexpr char kStartMediaSessionFailed[] = "Failed to start media session.";
inline constexpr char kMediaConnectedSignalingOffline[] =
    "Media connected, signaling offline";
inline constexpr char kSessionDisconnected[] = "Disconnected";
inline constexpr char kSignalingErrorPrefix[] = "Signaling error: ";

inline constexpr char kRoomApiConnectFailed[] = "Failed to connect room API";
inline constexpr char kRoomApiHttpErrorPrefix[] = "Room API returned HTTP ";
inline constexpr char kCreateRoomApiCallFailed[] = "Failed to call create room API";
inline constexpr char kCreateRoomInvalidResponse[] = "Invalid create room response";
inline constexpr char kCreateRoomMissingCode[] =
    "Create room response missing room code";
inline constexpr char kCreateRoomApiHttpErrorPrefix[] =
    "Create room API returned HTTP ";
inline constexpr char kRoomActionApiCallFailed[] = "Failed to call room action API";
inline constexpr char kRoomActionApiHttpErrorPrefix[] =
    "Room action API returned HTTP ";
inline constexpr char kIceApiConnectFailed[] = "Failed to connect ICE API";
inline constexpr char kIceApiHttpErrorPrefix[] = "ICE API returned HTTP ";
inline constexpr char kSignalingHandshakeFailed[] =
    "Signaling websocket handshake failed";
inline constexpr char kSignalingErrorFallback[] = "signaling error";

}  // namespace tempolink::juceapp::text
