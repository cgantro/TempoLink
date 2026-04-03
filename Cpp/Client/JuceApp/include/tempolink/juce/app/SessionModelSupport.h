#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "tempolink/client/ClientSession.h"
#include "tempolink/juce/network/ice/IceConfigTypes.h"
#include "tempolink/juce/ui/interfaces/ISessionView.h"
#include "tempolink/juce/ui/models/UiModels.h"

namespace tempolink::juceapp::app {

std::uint32_t HashToU32(const std::string& text);

void RebuildParticipants(const std::vector<std::string>& user_ids,
                         const std::string& current_user_id,
                         const std::string& selected_part_label, bool self_muted,
                         std::vector<ParticipantSummary>& out);

void RefreshSessionStatusView(ISessionView& session_view,
                              const std::string& active_room_code,
                              const tempolink::client::ClientSession::Stats& stats,
                              const std::string& audio_backend_name,
                              bool signaling_connected,
                              std::size_t participant_count,
                              const std::unordered_map<std::string, int>& peer_latency_ms,
                              bool ice_config_loaded,
                              const IceConfigSnapshot& ice_config);

void RefreshParticipantLevels(std::vector<ParticipantSummary>& participants,
                              ISessionView& session_view,
                              const tempolink::client::ClientSession& session,
                              const tempolink::client::ClientSession::Stats& stats,
                              bool signaling_connected,
                              const std::unordered_map<std::string, int>& peer_latency_ms,
                              bool ice_config_loaded,
                              const IceConfigSnapshot& ice_config);

}  // namespace tempolink::juceapp::app
