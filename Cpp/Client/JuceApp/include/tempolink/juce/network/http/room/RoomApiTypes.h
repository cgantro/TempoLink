#pragma once

#include <optional>
#include <string>
#include <vector>

#include <juce_core/juce_core.h>

struct RoomListFilter {
  std::string query;
  std::string tag;
  std::optional<bool> is_public;
  std::optional<bool> has_password;
  std::string mode = "all";
};

struct RoomUpdatePayload {
  juce::String name;
  juce::String description;
  std::vector<juce::String> tags;
  bool is_public = true;
  bool has_password = false;
  juce::String password;
  int max_participants = 6;
};

