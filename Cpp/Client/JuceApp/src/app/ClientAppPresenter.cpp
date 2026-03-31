#include "tempolink/juce/app/ClientAppPresenter.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include <juce_core/juce_core.h>

#include "tempolink/juce/app/ParticipantRosterBuilder.h"
#include "tempolink/juce/style/UiStyle.h"

ClientAppPresenter::ClientAppPresenter(LobbyView& lobby_view,
                                       RoomEntrySettingsView& room_entry_view,
                                       SessionView& session_view)
    : lobby_view_(lobby_view),
      room_entry_view_(room_entry_view),
      session_view_(session_view) {}

ClientAppPresenter::~ClientAppPresenter() { shutdown(); }

void ClientAppPresenter::initialize() {
  const int suffix = 100000 + juce::Random::getSystemRandom().nextInt(900000);
  current_user_id_ = "user-" + std::to_string(suffix);

  wireUiEvents();
  setLobbyStatusText("Loading rooms from control-plane...");
  fetchIceConfig();
  fetchRoomsFromApi();
  switchScreen(ScreenMode::Lobby);
}

void ClientAppPresenter::shutdown() {
  if (!alive_flag_->load()) {
    return;
  }
  alive_flag_->store(false);
  leaveRoom();
}

void ClientAppPresenter::tick() {
  ++tick_index_;

  if (screen_mode_ == ScreenMode::Lobby) {
    ++lobby_refresh_tick_;
    if (!room_fetch_in_flight_ &&
        lobby_refresh_tick_ >= tempolink::juceapp::style::kLobbyRefreshTicks) {
      lobby_refresh_tick_ = 0;
      fetchRoomsFromApi();
    }
  }

  if (!session_active_) {
    return;
  }

  session_.Tick();
  refreshSessionStatus();
  refreshParticipantLevels();
}

void ClientAppPresenter::fetchRoomsFromApi() {
  if (room_fetch_in_flight_) {
    return;
  }

  room_fetch_in_flight_ = true;
  auto alive = alive_flag_;
  room_api_.fetchRoomsAsync(
      [this, alive](bool ok, std::vector<RoomSummary> rooms, juce::String error_text) {
        if (!alive->load()) {
          return;
        }
        room_fetch_in_flight_ = false;
        if (!ok) {
          setLobbyStatusText("Room API unavailable: " + error_text);
          return;
        }

        rooms_ = std::move(rooms);
        lobby_view_.setRooms(rooms_);
        setLobbyStatusText("Loaded " +
                           juce::String(static_cast<int>(rooms_.size())) +
                           " rooms. User: " + juce::String(current_user_id_));
      });
}

void ClientAppPresenter::fetchIceConfig() {
  if (ice_fetch_in_flight_) {
    return;
  }

  ice_fetch_in_flight_ = true;
  auto alive = alive_flag_;
  ice_client_.fetchIceConfigAsync(
      [this, alive](bool ok, IceConfigSnapshot snapshot, juce::String error_text) {
        if (!alive->load()) {
          return;
        }
        ice_fetch_in_flight_ = false;
        if (!ok) {
          setLobbyStatusText("ICE config unavailable: " + error_text);
          return;
        }

        ice_config_ = std::move(snapshot);
        ice_config_loaded_ = true;
        const juce::String direct_hint = ice_config_.hasDirectAssist()
                                             ? "P2P assist ready"
                                             : "P2P assist limited";
        const juce::String fallback_hint = ice_config_.hasTurnFallback()
                                               ? "TURN fallback ready"
                                               : "TURN fallback unavailable";
        setLobbyStatusText("Network profile loaded (" + direct_hint + ", " +
                           fallback_hint + ")");
      });
}

void ClientAppPresenter::openRoomEntry(const std::string& room_code) {
  const RoomSummary* room = FindRoomByCode(rooms_, room_code);
  if (room == nullptr) {
    setLobbyStatusText("Room not found.");
    switchScreen(ScreenMode::Lobby);
    return;
  }

  entry_room_code_ = room_code;
  room_entry_view_.setRoom(*room);
  room_entry_view_.setSelectedPart(selected_part_label_);

  auto inputs = session_.AvailableInputDevices();
  auto outputs = session_.AvailableOutputDevices();
  std::string selected_input = preferred_input_device_;
  std::string selected_output = preferred_output_device_;

  if (selected_input.empty() && !inputs.empty()) {
    selected_input = inputs.front();
  }
  if (selected_output.empty() && !outputs.empty()) {
    selected_output = outputs.front();
  }

  room_entry_view_.setInputDevices(inputs, selected_input);
  room_entry_view_.setOutputDevices(outputs, selected_output);
  room_entry_view_.setStatusText("Check part and audio devices, then join.");
  switchScreen(ScreenMode::RoomEntry);
}

void ClientAppPresenter::openRoom(const std::string& room_code) {
  leaveRoom();

  const RoomSummary* room = FindRoomByCode(rooms_, room_code);
  if (room == nullptr) {
    setLobbyStatusText("Room not found.");
    return;
  }

  pending_room_code_ = room_code;
  const juce::String joining_text =
      "Joining room " + juce::String(room_code) + " ...";
  setLobbyStatusText(joining_text);
  room_entry_view_.setStatusText(joining_text);
  auto alive = alive_flag_;
  room_api_.joinRoomAsync(room_code, current_user_id_,
                          [this, alive, room_code](bool ok, juce::String error_text) {
                            if (!alive->load()) {
                              return;
                            }
                            if (pending_room_code_ != room_code) {
                              return;
                            }
                            if (!ok) {
                              pending_room_code_.clear();
                              const juce::String message = "Join failed: " + error_text;
                              setLobbyStatusText(message);
                              room_entry_view_.setStatusText(message);
                              return;
                            }

                            const RoomSummary* room_after_join =
                                FindRoomByCode(rooms_, room_code);
                            if (room_after_join == nullptr) {
                              pending_room_code_.clear();
                              const juce::String message =
                                  "Joined but room metadata missing.";
                              setLobbyStatusText(message);
                              room_entry_view_.setStatusText(message);
                              return;
                            }

                            tempolink::client::ClientSession::Config config;
                            config.server_host = room_after_join->relay_host;
                            config.server_port = room_after_join->relay_port;
                            config.room_id = HashToU32(room_code);
                            config.participant_id = HashToU32(current_user_id_);
                            config.nickname = current_user_id_;

                            if (!session_.Start(config) || !session_.Join()) {
                              session_.Stop();
                              pending_room_code_.clear();
                              const juce::String message =
                                  "Failed to start media session.";
                              setLobbyStatusText(message);
                              room_entry_view_.setStatusText(message);
                              return;
                            }

                            session_active_ = true;
                            active_room_code_ = room_code;
                            pending_room_code_.clear();
                            entry_room_code_.clear();

                            session_.SetMuted(false);
                            session_.SetMetronomeEnabled(false);
                            session_.SetMetronomeBpm(
                                tempolink::juceapp::style::kDefaultMetronomeBpm);
                            session_.SetVolume(
                                tempolink::juceapp::style::kDefaultMasterVolume);

                            if (!preferred_input_device_.empty()) {
                              session_.SetInputDevice(preferred_input_device_);
                            }
                            if (!preferred_output_device_.empty()) {
                              session_.SetOutputDevice(preferred_output_device_);
                            }

                            session_view_.setRoomTitle("Jam Session  |  Room " +
                                                       juce::String(room_code));
                            session_view_.setMute(false);
                            session_view_.setMetronomeEnabled(false);
                            session_view_.setMetronomeBpm(
                                tempolink::juceapp::style::kDefaultMetronomeBpm);
                            session_view_.setMasterVolume(
                                tempolink::juceapp::style::kDefaultMasterVolume);
                            setParticipantsFromUserIds(room_after_join->participants);
                            refreshSessionAudioDevices();
                            refreshSessionStatus();

                            const bool signaling_ok = signaling_client_.connect(
                                "127.0.0.1", 8080, room_code, current_user_id_,
                                [this, alive](const SignalingClient::Event& event) {
                                  if (!alive->load()) {
                                    return;
                                  }
                                  handleSignalingEvent(event);
                                });
                            if (!signaling_ok) {
                              session_view_.setStatusText(
                                  "Media connected, signaling offline");
                            }

                            switchScreen(ScreenMode::Session);
                          });
}

void ClientAppPresenter::leaveRoom() {
  signaling_client_.disconnect();

  if (session_active_ && !active_room_code_.empty()) {
    room_api_.leaveRoomAsync(active_room_code_, current_user_id_,
                             [](bool, juce::String) {});
  }

  if (session_active_) {
    session_.Leave();
    session_.Stop();
  }

  session_active_ = false;
  active_room_code_.clear();
  pending_room_code_.clear();
  participants_.clear();
  session_view_.setParticipants(participants_);
  session_view_.setConnectionState(false);
  session_view_.setStatusText("Disconnected");
}

void ClientAppPresenter::switchScreen(ScreenMode mode) {
  screen_mode_ = mode;
  const bool show_lobby = screen_mode_ == ScreenMode::Lobby;
  const bool show_room_entry = screen_mode_ == ScreenMode::RoomEntry;
  const bool show_session = screen_mode_ == ScreenMode::Session;
  lobby_view_.setVisible(show_lobby);
  room_entry_view_.setVisible(show_room_entry);
  session_view_.setVisible(show_session);
}

void ClientAppPresenter::refreshSessionAudioDevices() {
  if (!session_active_) {
    return;
  }
  session_view_.setInputDevices(session_.AvailableInputDevices(),
                                session_.SelectedInputDevice());
  session_view_.setOutputDevices(session_.AvailableOutputDevices(),
                                 session_.SelectedOutputDevice());
}

void ClientAppPresenter::refreshSessionStatus() {
  if (!session_active_) {
    return;
  }

  const auto& stats = session_.GetStats();
  session_view_.setConnectionState(stats.connected);
  ConnectionBadgeState route_state = ConnectionBadgeState::Reconnecting;
  if (stats.connected) {
    if (ice_config_loaded_ && ice_config_.hasDirectAssist() &&
        stats.last_rtt_ms > 0.0 && stats.last_rtt_ms < 26.0) {
      route_state = ConnectionBadgeState::P2P;
    } else {
      route_state = ConnectionBadgeState::Relay;
    }
  } else if (ice_config_loaded_ && !ice_config_.hasTurnFallback()) {
    route_state = ConnectionBadgeState::Failed;
  }
  session_view_.setConnectionMode(route_state);

  juce::String text = "Room " + juce::String(active_room_code_) + "  |  RTT " +
                      juce::String(static_cast<int>(stats.last_rtt_ms)) +
                      "ms  |  Offset " +
                      juce::String(static_cast<int>(stats.clock_offset_us)) + "us";
  text += "  |  " + juce::String(session_.AudioBackendName());
  text += "  |  route " + tempolink::juceapp::style::ConnectionBadgeText(route_state);
  session_view_.setStatusText(text);
}

void ClientAppPresenter::refreshParticipantLevels() {
  if (participants_.empty()) {
    return;
  }

  const auto& stats = session_.GetStats();
  ConnectionBadgeState room_connection_state = ConnectionBadgeState::Reconnecting;
  if (stats.connected) {
    if (ice_config_loaded_ && ice_config_.hasDirectAssist() &&
        stats.last_rtt_ms > 0.0 && stats.last_rtt_ms < 26.0) {
      room_connection_state = ConnectionBadgeState::P2P;
    } else {
      room_connection_state = ConnectionBadgeState::Relay;
    }
  } else if (ice_config_loaded_ && !ice_config_.hasTurnFallback()) {
    room_connection_state = ConnectionBadgeState::Failed;
  }

  for (std::size_t i = 0; i < participants_.size(); ++i) {
    auto& participant = participants_[i];
    if (participant.is_self) {
      participant.level = participant.is_muted
                              ? tempolink::juceapp::style::kMutedParticipantLevel
                              : AnimatedLevel(tick_index_, 0);
      participant.latency_ms = static_cast<int>(stats.last_rtt_ms);
      participant.packet_loss_percent = 0.0F;
      participant.connection_state = room_connection_state;
      continue;
    }
    participant.level = AnimatedLevel(tick_index_, static_cast<int>(i) * 31);
    participant.latency_ms = 15 + static_cast<int>((i * 7 + tick_index_) % 18);
    participant.packet_loss_percent =
        static_cast<float>(((i * 3 + tick_index_) % 8)) * 0.2F;
    participant.connection_state = room_connection_state;
  }
  session_view_.updateParticipantLevels(participants_);
}

void ClientAppPresenter::setLobbyStatusText(const juce::String& base_message) {
  juce::String message = base_message;
  if (ice_config_loaded_) {
    message += " | net: ";
    message += ice_config_.hasDirectAssist() ? "p2p-assist" : "limited-direct";
    message += ",";
    message += ice_config_.hasTurnFallback() ? "turn-ready" : "turn-missing";
  }
  lobby_view_.setStatusText(message);
}

void ClientAppPresenter::setParticipantsFromUserIds(
    const std::vector<std::string>& user_ids) {
  const bool self_muted = session_active_ ? session_.IsMuted() : false;
  participants_ =
      ParticipantRosterBuilder::Build(user_ids, current_user_id_, self_muted);
  for (auto& participant : participants_) {
    if (participant.is_self && !selected_part_label_.empty()) {
      participant.part_label = juce::String(selected_part_label_);
      break;
    }
  }
  session_view_.setParticipants(participants_);
}

void ClientAppPresenter::handleSignalingEvent(const SignalingClient::Event& event) {
  if (!session_active_) {
    return;
  }
  if (!event.room_code.empty() && event.room_code != active_room_code_) {
    return;
  }

  if (event.type == SignalingClient::Event::Type::RoomJoined) {
    setParticipantsFromUserIds(event.participants);
    return;
  }

  if (event.type == SignalingClient::Event::Type::PeerJoined &&
      !event.user_id.empty()) {
    setParticipantsFromUserIds(
        ParticipantRosterBuilder::WithJoined(participants_, event.user_id));
    return;
  }

  if (event.type == SignalingClient::Event::Type::PeerLeft &&
      !event.user_id.empty()) {
    setParticipantsFromUserIds(ParticipantRosterBuilder::WithoutUser(
        participants_, event.user_id, current_user_id_));
    return;
  }

  if (event.type == SignalingClient::Event::Type::Error) {
    session_view_.setStatusText("Signaling error: " + event.message);
  }
}

void ClientAppPresenter::wireUiEvents() {
  lobby_view_.setJoinHandler(
      [this](std::string room_code) { openRoomEntry(room_code); });

  room_entry_view_.setOnBack([this] {
    entry_room_code_.clear();
    setLobbyStatusText("Entry cancelled.");
    switchScreen(ScreenMode::Lobby);
  });

  room_entry_view_.setOnJoin(
      [this](RoomEntrySettingsView::EntrySelection selection) {
        selected_part_label_ = selection.part_label;
        preferred_input_device_ = selection.input_device;
        preferred_output_device_ = selection.output_device;
        const std::string room_code =
            selection.room_code.empty() ? entry_room_code_ : selection.room_code;
        if (room_code.empty()) {
          room_entry_view_.setStatusText("Room code missing.");
          return;
        }
        openRoom(room_code);
      });

  room_entry_view_.setOnPartChanged([this](std::string part_label) {
    selected_part_label_ = std::move(part_label);
  });

  room_entry_view_.setOnInputDeviceChanged([this](std::string input_device) {
    preferred_input_device_ = std::move(input_device);
  });

  room_entry_view_.setOnOutputDeviceChanged([this](std::string output_device) {
    preferred_output_device_ = std::move(output_device);
  });

  room_entry_view_.setOnOpenAudioSettings([this] {
    room_entry_view_.setStatusText("Advanced audio settings will open in-session.");
  });

  session_view_.setOnBack([this] {
    leaveRoom();
    setLobbyStatusText("Returned to lobby.");
    switchScreen(ScreenMode::Lobby);
  });

  session_view_.setOnDisconnect([this] {
    leaveRoom();
    setLobbyStatusText("Disconnected from room.");
    switchScreen(ScreenMode::Lobby);
  });

  session_view_.setOnMuteChanged([this](bool muted) {
    if (!session_active_) {
      return;
    }
    session_.SetMuted(muted);
    for (auto& participant : participants_) {
      if (participant.is_self) {
        participant.is_muted = muted;
        break;
      }
    }
    session_view_.setParticipants(participants_);
  });

  session_view_.setOnMetronomeChanged([this](bool enabled) {
    if (session_active_) {
      session_.SetMetronomeEnabled(enabled);
    }
  });

  session_view_.setOnVolumeChanged([this](float volume) {
    if (session_active_) {
      session_.SetVolume(volume);
    }
  });

  session_view_.setOnBpmChanged([this](int bpm) {
    if (session_active_) {
      session_.SetMetronomeBpm(bpm);
    }
  });

  session_view_.setOnInputDeviceChanged([this](std::string device_id) {
    if (session_active_) {
      session_.SetInputDevice(device_id);
    }
  });

  session_view_.setOnOutputDeviceChanged([this](std::string device_id) {
    if (session_active_) {
      session_.SetOutputDevice(device_id);
    }
  });

  session_view_.setOnOpenAudioSettings([this] {
    session_view_.setStatusText("Audio settings dialog will be connected next.");
  });

  session_view_.setOnParticipantAudioSettings([this](std::string user_id) {
    if (user_id.empty()) {
      return;
    }
    session_view_.setStatusText("Participant audio settings: " +
                                juce::String(user_id));
  });

  session_view_.setOnParticipantReconnect([this](std::string) {
    if (!session_active_ || active_room_code_.empty()) {
      return;
    }
    const std::string room_code = active_room_code_;
    leaveRoom();
    setLobbyStatusText("Reconnecting to room " + juce::String(room_code) + " ...");
    openRoom(room_code);
  });
}

std::uint32_t ClientAppPresenter::HashToU32(const std::string& text) {
  std::uint32_t hash = 2166136261u;
  for (unsigned char c : text) {
    hash ^= static_cast<std::uint32_t>(c);
    hash *= 16777619u;
  }
  return hash == 0 ? 1u : hash;
}

float ClientAppPresenter::AnimatedLevel(int tick_index, int phase_offset) {
  const float phase =
      static_cast<float>((tick_index + phase_offset) %
                         tempolink::juceapp::style::kAnimatedCycle) *
      tempolink::juceapp::style::kAnimatedPhaseStep;
  const float wave = 0.5F + 0.5F * std::sin(phase);
  return juce::jlimit(tempolink::juceapp::style::kAnimatedLevelMin,
                      tempolink::juceapp::style::kAnimatedLevelMax, wave);
}

const RoomSummary* ClientAppPresenter::FindRoomByCode(
    const std::vector<RoomSummary>& rooms,
    const std::string& room_code) {
  for (const auto& room : rooms) {
    if (room.room_code == room_code) {
      return &room;
    }
  }
  return nullptr;
}
