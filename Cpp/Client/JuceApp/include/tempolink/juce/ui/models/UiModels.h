#pragma once

#include <string>
#include <vector>

#include <optional>
#include "tempolink/juce/model/RoomSummary.h"

enum class ConnectionBadgeState {
  Unknown,
  P2P,
  Relay,
  Reconnecting,
  Failed
};

struct LobbyRoomFilter {
  std::string query;
  std::string tag;
  std::optional<bool> is_public;
  std::optional<bool> has_password;
  std::string mode = "all";
};

struct ParticipantSummary {
  std::string user_id;
  std::string display_name;
  std::string part_label;
  bool is_self = false;
  bool is_muted = false;
  bool is_recording = false;
  bool is_monitoring = false;
  float level = 0.0F;
  float monitor_volume = 1.0F;
  float monitor_pan = 0.0F;
  int latency_ms = 0;
  float packet_loss_percent = 0.0F;
  ConnectionBadgeState connection_state = ConnectionBadgeState::Unknown;
};
