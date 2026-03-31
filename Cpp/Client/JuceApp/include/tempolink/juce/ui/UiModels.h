#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

struct RoomSummary {
  std::string room_code;
  juce::String title;
  juce::String genre;
  std::string host_user_id;
  juce::String host_label;
  std::string relay_host = "127.0.0.1";
  std::uint16_t relay_port = 40000;
  int members = 0;
  int capacity = 0;
  juce::String latency_hint;
  std::vector<std::string> participants;
};

enum class ConnectionBadgeState {
  Unknown,
  P2P,
  Relay,
  Reconnecting,
  Failed
};

struct ParticipantSummary {
  std::string user_id;
  juce::String display_name;
  juce::String part_label;
  bool is_self = false;
  bool is_muted = false;
  bool is_recording = false;
  bool is_monitoring = false;
  float level = 0.0F;
  int latency_ms = 0;
  float packet_loss_percent = 0.0F;
  ConnectionBadgeState connection_state = ConnectionBadgeState::Unknown;
};
