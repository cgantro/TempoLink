#pragma once

#include <string>
#include <vector>

#include "tempolink/juce/ui/UiModels.h"

class ParticipantRosterBuilder {
 public:
  static std::vector<ParticipantSummary> Build(const std::vector<std::string>& user_ids,
                                               const std::string& current_user_id,
                                               bool self_muted);
  static std::vector<std::string> ToUserIds(
      const std::vector<ParticipantSummary>& participants);
  static std::vector<std::string> WithJoined(
      const std::vector<ParticipantSummary>& participants,
      const std::string& joined_user_id);
  static std::vector<std::string> WithoutUser(
      const std::vector<ParticipantSummary>& participants,
      const std::string& removed_user_id,
      const std::string& current_user_id);
};
