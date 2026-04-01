#pragma once

#include <string>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "tempolink/juce/model/RoomSummary.h"

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
  float monitor_volume = 1.0F;
  float monitor_pan = 0.0F;
  int latency_ms = 0;
  float packet_loss_percent = 0.0F;
  ConnectionBadgeState connection_state = ConnectionBadgeState::Unknown;
};
