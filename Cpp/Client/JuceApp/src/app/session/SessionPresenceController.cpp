#include "tempolink/juce/app/session/SessionPresenceController.h"

#include <chrono>
#include <utility>

#include <juce_core/juce_core.h>

#include "tempolink/juce/app/ParticipantRosterBuilder.h"
#include "tempolink/juce/app/SessionModelSupport.h"

SessionPresenceController::SessionPresenceController(
    SessionView& session_view, tempolink::client::ClientSession& session,
    SignalingClient& signaling_client)
    : session_view_(session_view),
      session_(session),
      signaling_client_(signaling_client) {}

void SessionPresenceController::reset() {
  participants_.clear();
  peer_latency_ms_.clear();
  peer_ping_last_sent_ms_.clear();
  last_peer_ping_tick_ = std::chrono::steady_clock::time_point::min();
  last_device_refresh_tick_ = std::chrono::steady_clock::time_point::min();
  session_view_.setParticipants(participants_);
}

void SessionPresenceController::initializeParticipants(
    const std::vector<std::string>& user_ids, const std::string& current_user_id,
    const std::string& selected_part_label, bool self_muted) {
  rebuildParticipantsFromUserIds(user_ids, current_user_id, selected_part_label,
                                 self_muted);
}

void SessionPresenceController::updateSelfMuted(bool muted) {
  for (auto& participant : participants_) {
    if (participant.is_self) {
      participant.is_muted = muted;
      break;
    }
  }
  session_view_.setParticipants(participants_);
}

void SessionPresenceController::updateParticipantMonitorVolume(
    const std::string& user_id, float volume) {
  for (auto& participant : participants_) {
    if (participant.user_id != user_id) {
      continue;
    }
    participant.monitor_volume = juce::jlimit(0.0F, 1.5F, volume);
    session_view_.setParticipantMonitorVolume(user_id, participant.monitor_volume);
    break;
  }
}

void SessionPresenceController::updateParticipantMonitorPan(
    const std::string& user_id, float pan) {
  for (auto& participant : participants_) {
    if (participant.user_id != user_id) {
      continue;
    }
    participant.monitor_pan = juce::jlimit(-1.0F, 1.0F, pan);
    session_view_.setParticipantMonitorPan(user_id, participant.monitor_pan);
    break;
  }
}

void SessionPresenceController::refreshAudioDevices(bool session_active) {
  if (!session_active) {
    return;
  }
  session_view_.setInputDevices(session_.AvailableInputDevices(),
                                session_.SelectedInputDevice());
  session_view_.setOutputDevices(session_.AvailableOutputDevices(),
                                 session_.SelectedOutputDevice());
}

void SessionPresenceController::refreshViewState(bool session_active,
                                                 const std::string& active_room_code,
                                                 int tick_index,
                                                 bool signaling_connected,
                                                 bool ice_config_loaded,
                                                 const IceConfigSnapshot& ice_config) {
  if (!session_active) {
    return;
  }
  (void)tick_index;

  const auto now = std::chrono::steady_clock::now();
  if (last_device_refresh_tick_ == std::chrono::steady_clock::time_point::min() ||
      now - last_device_refresh_tick_ >= std::chrono::seconds(2)) {
    refreshAudioDevices(session_active);
    last_device_refresh_tick_ = now;
  }

  const auto& stats = session_.GetStats();
  session_view_.setInputLevel(session_.InputLevel());
  tempolink::juceapp::app::RefreshSessionStatusView(
      session_view_, active_room_code, stats, session_.AudioBackendName(),
      signaling_connected, participants_.size(), peer_latency_ms_,
      ice_config_loaded, ice_config);
  tempolink::juceapp::app::RefreshParticipantLevels(
      participants_, session_view_, session_, stats, signaling_connected,
      peer_latency_ms_, ice_config_loaded, ice_config);
}

void SessionPresenceController::sendPeerPings(bool session_active) {
  if (!session_active || !signaling_client_.isConnected()) {
    return;
  }
  const auto now = std::chrono::steady_clock::now();
  if (last_peer_ping_tick_ != std::chrono::steady_clock::time_point::min() &&
      now - last_peer_ping_tick_ < std::chrono::milliseconds(1000)) {
    return;
  }
  last_peer_ping_tick_ = now;
  const std::uint64_t sent_at_ms = nowSteadyMs();
  for (const auto& participant : participants_) {
    if (participant.is_self || participant.user_id.empty()) {
      continue;
    }
    if (signaling_client_.sendPeerPing(participant.user_id, sent_at_ms)) {
      peer_ping_last_sent_ms_[participant.user_id] = sent_at_ms;
    }
  }
}

bool SessionPresenceController::sendReconnectProbe(const std::string& user_id,
                                                   bool session_active) {
  if (!session_active || !signaling_client_.isConnected() || user_id.empty()) {
    return false;
  }
  const std::uint64_t sent_at_ms = nowSteadyMs();
  if (!signaling_client_.sendPeerPing(user_id, sent_at_ms)) {
    return false;
  }
  peer_ping_last_sent_ms_[user_id] = sent_at_ms;
  return true;
}

void SessionPresenceController::handleSignalingEvent(
    bool session_active, const std::string& active_room_code,
    const std::string& current_user_id, const std::string& selected_part_label,
    const SignalingClient::Event& event) {
  if (!session_active) {
    return;
  }
  if (!event.room_code.empty() && event.room_code != active_room_code) {
    return;
  }

  if (event.type == SignalingClient::Event::Type::RoomJoined) {
    rebuildParticipantsFromUserIds(event.participants, current_user_id,
                                   selected_part_label, session_.IsMuted());
    return;
  }

  if (event.type == SignalingClient::Event::Type::PeerJoined &&
      !event.user_id.empty()) {
    rebuildParticipantsFromUserIds(
        ParticipantRosterBuilder::WithJoined(participants_, event.user_id),
        current_user_id, selected_part_label, session_.IsMuted());
    return;
  }

  if (event.type == SignalingClient::Event::Type::PeerLeft &&
      !event.user_id.empty()) {
    peer_latency_ms_.erase(event.user_id);
    peer_ping_last_sent_ms_.erase(event.user_id);
    rebuildParticipantsFromUserIds(
        ParticipantRosterBuilder::WithoutUser(
            participants_, event.user_id, current_user_id),
        current_user_id, selected_part_label, session_.IsMuted());
    return;
  }

  if (event.type == SignalingClient::Event::Type::PeerPing) {
    if (!event.from_user_id.empty() && event.sent_at_ms > 0) {
      signaling_client_.sendPeerPong(event.from_user_id, event.sent_at_ms);
    }
    return;
  }

  if (event.type == SignalingClient::Event::Type::PeerPong) {
    if (!event.from_user_id.empty() && event.sent_at_ms > 0) {
      const std::uint64_t now_ms = nowSteadyMs();
      if (now_ms >= event.sent_at_ms) {
        const auto rtt_ms = static_cast<int>(now_ms - event.sent_at_ms);
        peer_latency_ms_[event.from_user_id] = juce::jlimit(0, 5000, rtt_ms);
      }
    }
    return;
  }

  if (event.type == SignalingClient::Event::Type::Error) {
    session_view_.setStatusText(juce::String("Signaling error: ") +
                                event.message);
  }
}

const std::vector<ParticipantSummary>& SessionPresenceController::participants()
    const {
  return participants_;
}

void SessionPresenceController::rebuildParticipantsFromUserIds(
    const std::vector<std::string>& user_ids, const std::string& current_user_id,
    const std::string& selected_part_label, bool self_muted) {
  std::unordered_map<std::string, std::pair<float, float>> mix_by_user;
  mix_by_user.reserve(participants_.size());
  for (const auto& participant : participants_) {
    mix_by_user[participant.user_id] =
        std::make_pair(participant.monitor_volume, participant.monitor_pan);
  }

  tempolink::juceapp::app::RebuildParticipants(
      user_ids, current_user_id, selected_part_label, self_muted, participants_);
  for (auto& participant : participants_) {
    const auto it = mix_by_user.find(participant.user_id);
    if (it == mix_by_user.end()) {
      continue;
    }
    participant.monitor_volume = juce::jlimit(0.0F, 1.5F, it->second.first);
    participant.monitor_pan = juce::jlimit(-1.0F, 1.0F, it->second.second);
  }
  session_view_.setParticipants(participants_);
}

std::uint64_t SessionPresenceController::nowSteadyMs() {
  return static_cast<std::uint64_t>(
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now().time_since_epoch())
          .count());
}
