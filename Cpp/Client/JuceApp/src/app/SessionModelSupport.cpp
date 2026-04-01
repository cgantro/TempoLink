#include "tempolink/juce/app/SessionModelSupport.h"

#include <cmath>

#include <juce_core/juce_core.h>

#include "tempolink/juce/app/ParticipantRosterBuilder.h"
#include "tempolink/juce/style/UiStyle.h"

namespace tempolink::juceapp::app {
namespace {

float AnimatedLevel(int tick_index, int phase_offset) {
  const float phase =
      static_cast<float>((tick_index + phase_offset) %
                         tempolink::juceapp::style::kAnimatedCycle) *
      tempolink::juceapp::style::kAnimatedPhaseStep;
  const float wave = 0.5F + 0.5F * std::sin(phase);
  return juce::jlimit(tempolink::juceapp::style::kAnimatedLevelMin,
                      tempolink::juceapp::style::kAnimatedLevelMax, wave);
}

ConnectionBadgeState ResolveConnectionState(
    const tempolink::client::ClientSession::Stats& stats, bool ice_config_loaded,
    const IceConfigSnapshot& ice_config, bool signaling_connected,
    std::size_t participant_count,
    const std::unordered_map<std::string, int>& peer_latency_ms) {
  bool has_peer_rtt = false;
  for (const auto& [_, latency_ms] : peer_latency_ms) {
    if (latency_ms >= 0) {
      has_peer_rtt = true;
      break;
    }
  }

  if (has_peer_rtt) {
    return ConnectionBadgeState::P2P;
  }

  if (stats.connected) {
    return ConnectionBadgeState::Relay;
  }

  if (!signaling_connected) {
    return ConnectionBadgeState::Failed;
  }

  if (participant_count <= 1) {
    return ConnectionBadgeState::Unknown;
  }

  (void)ice_config_loaded;
  (void)ice_config;
  return ConnectionBadgeState::Reconnecting;
}

}  // namespace

std::uint32_t HashToU32(const std::string& text) {
  std::uint32_t hash = 2166136261u;
  for (unsigned char c : text) {
    hash ^= static_cast<std::uint32_t>(c);
    hash *= 16777619u;
  }
  return hash == 0 ? 1u : hash;
}

void RebuildParticipants(const std::vector<std::string>& user_ids,
                         const std::string& current_user_id,
                         const std::string& selected_part_label, bool self_muted,
                         std::vector<ParticipantSummary>& out) {
  out = ParticipantRosterBuilder::Build(user_ids, current_user_id, self_muted);
  for (auto& participant : out) {
    if (participant.is_self && !selected_part_label.empty()) {
      participant.part_label = juce::String(selected_part_label);
      break;
    }
  }
}

void RefreshSessionStatusView(SessionView& session_view,
                              const std::string& active_room_code,
                              const tempolink::client::ClientSession::Stats& stats,
                              const std::string& audio_backend_name,
                              bool signaling_connected,
                              std::size_t participant_count,
                              const std::unordered_map<std::string, int>& peer_latency_ms,
                              bool ice_config_loaded,
                              const IceConfigSnapshot& ice_config) {
  const ConnectionBadgeState route_state =
      ResolveConnectionState(stats, ice_config_loaded, ice_config,
                             signaling_connected, participant_count,
                             peer_latency_ms);

  session_view.setConnectionState(route_state == ConnectionBadgeState::P2P);
  session_view.setConnectionMode(route_state);

  juce::String text = "Room " + juce::String(active_room_code) + "  |  RTT " +
                      juce::String(static_cast<int>(stats.last_rtt_ms)) +
                      "ms  |  Offset " +
                      juce::String(static_cast<int>(stats.clock_offset_us)) + "us";
  text += "  |  " + juce::String(audio_backend_name);
  text += "  |  route " + tempolink::juceapp::style::ConnectionBadgeText(route_state);
  if (route_state == ConnectionBadgeState::Unknown && participant_count <= 1) {
    text += "  |  waiting for peer";
  } else if (route_state == ConnectionBadgeState::Failed) {
    text += "  |  signaling disconnected";
  }
  session_view.setStatusText(text);
}

void RefreshParticipantLevels(std::vector<ParticipantSummary>& participants,
                              SessionView& session_view, int tick_index,
                              const tempolink::client::ClientSession::Stats& stats,
                              bool signaling_connected,
                              const std::unordered_map<std::string, int>& peer_latency_ms,
                              bool ice_config_loaded,
                              const IceConfigSnapshot& ice_config) {
  if (participants.empty()) {
    return;
  }

  const ConnectionBadgeState room_connection_state =
      ResolveConnectionState(stats, ice_config_loaded, ice_config,
                             signaling_connected, participants.size(),
                             peer_latency_ms);
  for (std::size_t i = 0; i < participants.size(); ++i) {
    auto& participant = participants[i];
    if (participant.is_self) {
      participant.level = participant.is_muted
                              ? tempolink::juceapp::style::kMutedParticipantLevel
                              : AnimatedLevel(tick_index, 0);
      participant.latency_ms = static_cast<int>(stats.last_rtt_ms);
      participant.packet_loss_percent = 0.0F;
      participant.connection_state = room_connection_state;
      continue;
    }

    participant.level = AnimatedLevel(tick_index, static_cast<int>(i) * 31);
    const auto latency_it = peer_latency_ms.find(participant.user_id);
    participant.latency_ms =
        latency_it == peer_latency_ms.end() ? -1 : latency_it->second;
    participant.packet_loss_percent = 0.0F;
    if (participant.latency_ms >= 0) {
      participant.connection_state = ConnectionBadgeState::P2P;
    } else if (!signaling_connected) {
      participant.connection_state = ConnectionBadgeState::Failed;
    } else {
      participant.connection_state = room_connection_state;
    }
  }
  session_view.updateParticipantLevels(participants);
}

}  // namespace tempolink::juceapp::app
