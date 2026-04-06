#include "tempolink/juce/app/session/SessionPresenceController.h"

#include <chrono>
#include <functional>
#include <unordered_map>
#include <utility>

#include <juce_core/juce_core.h>

#include "tempolink/juce/app/ParticipantRosterBuilder.h"
#include "tempolink/juce/app/SessionModelSupport.h"
#include "tempolink/juce/logging/AppLogger.h"

// ---------------------------------------------------------------------------
// Event dispatch table type
// ---------------------------------------------------------------------------
namespace {

struct EventContext {
  SessionPresenceController& self;
  const std::string& current_user_id;
  const std::string& selected_part_label;
  const SignalingClient::Event& event;
};

using EventHandler = std::function<void(EventContext& ctx)>;

}  // namespace

// ---------------------------------------------------------------------------
// Construction / reset
// ---------------------------------------------------------------------------

SessionPresenceController::SessionPresenceController(
    ISessionView& session_view, tempolink::client::ClientSession& session,
    SignalingClient& signaling_client)
    : session_view_(session_view),
      session_(session),
      signaling_client_(signaling_client) {
  // Original implementation had session_view_.setSignalingClient(signaling_client_);
  // but a pure interface shouldn't know about concrete SignalingClient.
  // We'll move that responsibility to the Coordinator or keep it if the interface allows.
  // For now, ISessionView doesn't have it, so we skip it here.
}

void SessionPresenceController::reset() {
  participants_.clear();
  peer_latency_ms_.clear();
  peer_ping_last_sent_ms_.clear();
  has_last_signaling_connected_ = false;
  last_signaling_connected_ = false;
  has_last_media_connected_ = false;
  last_media_connected_ = false;
  last_peer_rtt_count_ = -1;
  last_peer_ping_tick_ = std::chrono::steady_clock::time_point::min();
  last_device_refresh_tick_ = std::chrono::steady_clock::time_point::min();
  session_view_.setParticipants(participants_);
}

// ---------------------------------------------------------------------------
// Participant management
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// Device / view refresh
// ---------------------------------------------------------------------------

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
  int peer_rtt_count = 0;
  for (const auto& [_, latency_ms] : peer_latency_ms_) {
    if (latency_ms >= 0) {
      ++peer_rtt_count;
    }
  }

  if (!has_last_signaling_connected_ ||
      last_signaling_connected_ != signaling_connected) {
    tempolink::juceapp::logging::Info(
        "Session signaling state changed: " +
        juce::String(signaling_connected ? "connected" : "disconnected"));
    has_last_signaling_connected_ = true;
    last_signaling_connected_ = signaling_connected;
  }

  if (!has_last_media_connected_ || last_media_connected_ != stats.connected) {
    tempolink::juceapp::logging::Info(
        "Session media RTT state changed: " +
        juce::String(stats.connected ? "relay-rtt-ok" : "relay-rtt-missing"));
    has_last_media_connected_ = true;
    last_media_connected_ = stats.connected;
  }

  if (last_peer_rtt_count_ != peer_rtt_count) {
    tempolink::juceapp::logging::Info(
        "Session peer RTT count changed: " + juce::String(peer_rtt_count) +
        " (p2p-active=" + juce::String(peer_rtt_count > 0 ? "true" : "false") +
        ")");
    last_peer_rtt_count_ = peer_rtt_count;
  }

  session_view_.setInputLevel(session_.InputLevel());
  tempolink::juceapp::app::RefreshSessionStatusView(
      session_view_, active_room_code, stats, session_.AudioBackendName(),
      signaling_connected, static_cast<int>(participants_.size()), peer_latency_ms_,
      ice_config_loaded, ice_config);
  tempolink::juceapp::app::RefreshParticipantLevels(
      participants_, session_view_, session_, stats, signaling_connected,
      peer_latency_ms_, ice_config_loaded, ice_config);
}

// ---------------------------------------------------------------------------
// Peer ping
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// Signaling event dispatch — dispatch table replaces if-else chain
// ---------------------------------------------------------------------------

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

  using Type = SignalingClient::Event::Type;
  if (event.type == Type::Error) {
    tempolink::juceapp::logging::Warn(
        "Signaling event error: " + juce::String(event.message));
  } else if (event.type == Type::PeerJoined || event.type == Type::PeerLeft ||
             event.type == Type::RoomJoined) {
    tempolink::juceapp::logging::Info(
        "Signaling event in session: type=" +
        juce::String(event.type == Type::RoomJoined
                         ? "room.joined"
                         : (event.type == Type::PeerJoined ? "peer.joined"
                                                            : "peer.left")) +
        ", user=" + juce::String(event.user_id) +
        ", participants=" + juce::String(static_cast<int>(event.participants.size())));
  }

  // Static dispatch table — maps event type to handler logic.
  static const std::unordered_map<Type, EventHandler> dispatch = {
      {Type::RoomJoined, [](EventContext& ctx) {
        ctx.self.rebuildParticipantsFromUserIds(
            ctx.event.participants, ctx.current_user_id,
            ctx.selected_part_label, ctx.self.session_.IsMuted());
      }},
      {Type::PeerJoined, [](EventContext& ctx) {
        if (ctx.event.user_id.empty()) return;
        ctx.self.rebuildParticipantsFromUserIds(
            ParticipantRosterBuilder::WithJoined(ctx.self.participants_, ctx.event.user_id),
            ctx.current_user_id, ctx.selected_part_label, ctx.self.session_.IsMuted());
      }},
      {Type::PeerLeft, [](EventContext& ctx) {
        if (ctx.event.user_id.empty()) return;
        ctx.self.peer_latency_ms_.erase(ctx.event.user_id);
        ctx.self.peer_ping_last_sent_ms_.erase(ctx.event.user_id);
        ctx.self.rebuildParticipantsFromUserIds(
            ParticipantRosterBuilder::WithoutUser(
                ctx.self.participants_, ctx.event.user_id, ctx.current_user_id),
            ctx.current_user_id, ctx.selected_part_label, ctx.self.session_.IsMuted());
      }},
      {Type::PeerPing, [](EventContext& ctx) {
        if (!ctx.event.from_user_id.empty() && ctx.event.sent_at_ms > 0) {
          ctx.self.signaling_client_.sendPeerPong(ctx.event.from_user_id, ctx.event.sent_at_ms);
        }
      }},
      {Type::PeerPong, [](EventContext& ctx) {
        if (!ctx.event.from_user_id.empty() && ctx.event.sent_at_ms > 0) {
          const std::uint64_t now_ms = SessionPresenceController::nowSteadyMs();
          if (now_ms >= ctx.event.sent_at_ms) {
            const auto rtt_ms = static_cast<int>(now_ms - ctx.event.sent_at_ms);
            ctx.self.peer_latency_ms_[ctx.event.from_user_id] = juce::jlimit(0, 5000, rtt_ms);
          }
        }
      }},
      {Type::ChatMessage, [](EventContext& ctx) {
        ctx.self.session_view_.addChatMessage(ctx.event.from_user_id, ctx.event.message.toStdString(), false);
      }},
      {Type::Error, [](EventContext& ctx) {
        ctx.self.session_view_.setStatusText("Signaling error: " + ctx.event.message.toStdString());
      }},
  };

  const auto it = dispatch.find(event.type);
  if (it != dispatch.end()) {
    EventContext ctx{*this, current_user_id, selected_part_label, event};
    it->second(ctx);
  }
}

// ---------------------------------------------------------------------------
// Participant list
// ---------------------------------------------------------------------------

const std::vector<ParticipantSummary>& SessionPresenceController::participants() const {
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
