#include "tempolink/juce/app/ParticipantRosterBuilder.h"

#include <algorithm>
#include <utility>

std::vector<ParticipantSummary> ParticipantRosterBuilder::Build(
    const std::vector<std::string>& user_ids,
    const std::string& current_user_id,
    bool self_muted) {
  std::vector<std::string> unique_ids = user_ids;
  unique_ids.erase(std::remove_if(unique_ids.begin(), unique_ids.end(),
                                  [](const std::string& id) { return id.empty(); }),
                   unique_ids.end());
  std::sort(unique_ids.begin(), unique_ids.end());
  unique_ids.erase(std::unique(unique_ids.begin(), unique_ids.end()), unique_ids.end());
  if (std::find(unique_ids.begin(), unique_ids.end(), current_user_id) ==
      unique_ids.end()) {
    unique_ids.push_back(current_user_id);
  }

  std::vector<ParticipantSummary> participants;
  participants.reserve(unique_ids.size());
  for (const auto& id : unique_ids) {
    ParticipantSummary summary;
    summary.user_id = id;
    summary.is_self = id == current_user_id;
    summary.part_label = summary.is_self ? juce::String("My Part")
                                         : juce::String("Session Part");
    summary.is_muted = summary.is_self ? self_muted : false;
    summary.level = 0.0F;
    summary.latency_ms = 0;
    summary.packet_loss_percent = 0.0F;
    summary.connection_state = ConnectionBadgeState::Unknown;
    summary.display_name = summary.is_self ? juce::String("You") : juce::String(id);
    participants.push_back(std::move(summary));
  }

  std::stable_sort(participants.begin(), participants.end(),
                   [](const ParticipantSummary& a, const ParticipantSummary& b) {
                     if (a.is_self != b.is_self) {
                       return a.is_self;
                     }
                     return a.user_id < b.user_id;
                   });
  return participants;
}

std::vector<std::string> ParticipantRosterBuilder::ToUserIds(
    const std::vector<ParticipantSummary>& participants) {
  std::vector<std::string> ids;
  ids.reserve(participants.size());
  for (const auto& participant : participants) {
    ids.push_back(participant.user_id);
  }
  return ids;
}

std::vector<std::string> ParticipantRosterBuilder::WithJoined(
    const std::vector<ParticipantSummary>& participants,
    const std::string& joined_user_id) {
  std::vector<std::string> ids = ToUserIds(participants);
  ids.push_back(joined_user_id);
  return ids;
}

std::vector<std::string> ParticipantRosterBuilder::WithoutUser(
    const std::vector<ParticipantSummary>& participants,
    const std::string& removed_user_id,
    const std::string& current_user_id) {
  std::vector<std::string> ids;
  ids.reserve(participants.size());
  for (const auto& participant : participants) {
    if (participant.user_id == removed_user_id && participant.user_id != current_user_id) {
      continue;
    }
    ids.push_back(participant.user_id);
  }
  return ids;
}
