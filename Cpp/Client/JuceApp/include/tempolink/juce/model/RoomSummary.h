#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

struct RoomSummary {
  std::string room_code;
  juce::String title;
  juce::String description;
  juce::String genre;
  std::vector<juce::String> tags;
  std::string host_user_id;
  juce::String host_label;
  std::string relay_host;
  std::uint16_t relay_port = 0;
  bool is_public = true;
  bool has_password = false;
  bool is_active = false;
  int members = 0;
  int capacity = 0;
  juce::String latency_hint;
  std::vector<std::string> participants;
};
