#include "tempolink/juce/app/session/SessionLifecycleController.h"

#include <utility>

#include <juce_core/juce_core.h>

#include "tempolink/juce/app/SessionModelSupport.h"
#include "tempolink/juce/constants/ClientText.h"
#include "tempolink/juce/logging/AppLogger.h"
#include "tempolink/juce/style/UiStyle.h"

SessionLifecycleController::SessionLifecycleController(
    SessionServices services, AppStatusContext& status_context,
    SessionViewCallbacks view_callbacks,
    SessionRoutingCallbacks routing_callbacks,
    std::shared_ptr<std::atomic_bool> alive_flag)
    : session_(services.session),
      room_api_(services.room_api),
      signaling_client_(services.signaling_client),
      session_presence_controller_(services.session_presence_controller),
      room_catalog_(services.room_catalog),
      alive_flag_(std::move(alive_flag)),
      view_callbacks_(std::move(view_callbacks)),
      routing_callbacks_(std::move(routing_callbacks)),
      status_context_(status_context) {}

void SessionLifecycleController::OpenRoomEntry(const std::string& room_code) {
  const RoomSummary* room = room_catalog_.findByCode(room_code);
  if (room == nullptr) {
    if (routing_callbacks_.set_lobby_status_text) {
      routing_callbacks_.set_lobby_status_text(
          std::string(tempolink::juceapp::text::kRoomNotFound));
    }
    if (routing_callbacks_.switch_screen) {
      routing_callbacks_.switch_screen(tempolink::juceapp::app::ScreenMode::Lobby);
    }
    return;
  }

  entry_room_code_ = room_code;
  if (view_callbacks_.set_room_entry_room) {
    view_callbacks_.set_room_entry_room(*room);
  }
  if (view_callbacks_.set_room_entry_selected_part) {
    view_callbacks_.set_room_entry_selected_part(status_context_.selected_part_label);
  }

  auto inputs = session_.AvailableInputDevices();
  auto outputs = session_.AvailableOutputDevices();
  std::string selected_input = status_context_.preferred_input_device;
  std::string selected_output = status_context_.preferred_output_device;

  if (selected_input.empty() && !inputs.empty()) {
    selected_input = inputs.front();
  }
  if (selected_output.empty() && !outputs.empty()) {
    selected_output = outputs.front();
  }

  if (view_callbacks_.set_room_entry_input_devices) {
    view_callbacks_.set_room_entry_input_devices(inputs, selected_input);
  }
  if (view_callbacks_.set_room_entry_output_devices) {
    view_callbacks_.set_room_entry_output_devices(outputs, selected_output);
  }
  if (inputs.empty() || outputs.empty()) {
    if (view_callbacks_.set_room_entry_status_text) {
      view_callbacks_.set_room_entry_status_text(
          "오디오 장치 없음: 입력/출력 장치를 연결한 뒤 다시 시도하세요.");
    }
  } else {
    if (view_callbacks_.set_room_entry_status_text) {
      view_callbacks_.set_room_entry_status_text(
          std::string(tempolink::juceapp::text::kEntryPreflightGuide));
    }
  }
  if (routing_callbacks_.switch_screen) {
    routing_callbacks_.switch_screen(tempolink::juceapp::app::ScreenMode::RoomEntry);
  }
}

void SessionLifecycleController::PreviewRoom(const std::string& room_code) {
  OpenRoomEntry(room_code);
  if (view_callbacks_.set_room_entry_status_text) {
    view_callbacks_.set_room_entry_status_text(
        "PREVIEW selected. Monitor-only mode will be enabled in next step.");
  }
}

void SessionLifecycleController::OpenRoom(const std::string& room_code,
                                          int tick_index, bool ice_config_loaded,
                                          const IceConfigSnapshot& ice_config) {
  tempolink::juceapp::logging::Info(
      "OpenRoom requested: room=" + juce::String(room_code) +
      ", user=" + juce::String(status_context_.current_user_id));

  if (!status_context_.auth_completed || status_context_.current_user_id.empty()) {
    if (status_context_.current_user_id.empty()) {
      const int guest_suffix = 100000 + juce::Random::getSystemRandom().nextInt(900000);
      status_context_.current_user_id = "local-guest-" + std::to_string(guest_suffix);
    }
    if (status_context_.current_display_name.empty()) {
      status_context_.current_display_name = status_context_.current_user_id;
    }
    status_context_.auth_completed = true;
  }
  if (!status_context_.auth_completed || status_context_.current_user_id.empty()) {
    tempolink::juceapp::logging::Error(
        "OpenRoom blocked by auth guard: auth_completed=" +
        juce::String(status_context_.auth_completed ? "true" : "false") +
        ", user_id=" + juce::String(status_context_.current_user_id));
    if (view_callbacks_.set_login_status_text) {
      view_callbacks_.set_login_status_text("Please sign in before joining rooms.");
    }
    if (view_callbacks_.set_room_entry_status_text) {
      view_callbacks_.set_room_entry_status_text(
          "로그인 후 입장할 수 있습니다. 로그인 화면으로 이동합니다.");
    }
    if (routing_callbacks_.switch_screen) {
      routing_callbacks_.switch_screen(tempolink::juceapp::app::ScreenMode::Login);
    }
    return;
  }

  LeaveRoom();

  const RoomSummary* room = room_catalog_.findByCode(room_code);
  if (room == nullptr) {
    if (routing_callbacks_.set_lobby_status_text) {
      routing_callbacks_.set_lobby_status_text(
          std::string(tempolink::juceapp::text::kRoomNotFound));
    }
    return;
  }
  const RoomSummary room_snapshot = *room;

  pending_room_code_ = room_code;
  const std::string joining_text =
      (juce::String(tempolink::juceapp::text::kJoiningRoomPrefix) +
       juce::String(room_code) + tempolink::juceapp::text::kJoinPendingSuffix)
          .toStdString();
  if (routing_callbacks_.set_lobby_status_text) {
    routing_callbacks_.set_lobby_status_text(joining_text);
  }
  if (view_callbacks_.set_room_entry_status_text) {
    view_callbacks_.set_room_entry_status_text(joining_text);
  }

  auto alive = alive_flag_;
  room_api_.joinRoomAsync(room_code, status_context_.current_user_id,
                          [this, alive, room_code, tick_index, ice_config_loaded,
                           ice_config, room_snapshot](bool ok, juce::String error_text) {
                            if (!alive->load()) {
                              return;
                            }
                            if (pending_room_code_ != room_code) {
                              return;
                            }
                            if (!ok) {
                              pending_room_code_.clear();
                              const std::string message =
                                  (juce::String(tempolink::juceapp::text::kJoinFailedPrefix) +
                                   error_text)
                                      .toStdString();
                              if (routing_callbacks_.set_lobby_status_text) {
                                routing_callbacks_.set_lobby_status_text(message);
                              }
                              if (view_callbacks_.set_room_entry_status_text) {
                                view_callbacks_.set_room_entry_status_text(message);
                              }
                              return;
                            }

                            tempolink::client::ClientSession::Config config;
                            config.server_host = room_snapshot.relay_host.empty()
                                                     ? status_context_.default_relay_host
                                                     : room_snapshot.relay_host;
                            config.server_port = room_snapshot.relay_port == 0
                                                     ? status_context_.default_relay_port
                                                     : room_snapshot.relay_port;
                            config.room_id = tempolink::juceapp::app::HashToU32(room_code);
                            config.participant_id =
                                tempolink::juceapp::app::HashToU32(status_context_.current_user_id);
                            config.nickname =
                                status_context_.current_display_name.empty()
                                    ? status_context_.current_user_id
                                    : status_context_.current_display_name;

                            tempolink::juceapp::logging::Info(
                                "Starting media session: relay=" +
                                juce::String(config.server_host) + ":" +
                                juce::String(static_cast<int>(config.server_port)) +
                                ", participantId=" +
                                juce::String(static_cast<int>(config.participant_id)));

                            if (!session_.Start(config) || !session_.Join()) {
                              tempolink::juceapp::logging::Error(
                                  "Media session start/join failed roomCode=" +
                                  juce::String(room_code));
                              session_.Stop();
                              pending_room_code_.clear();
                              const std::string message =
                                  std::string(tempolink::juceapp::text::kStartMediaSessionFailed);
                              if (routing_callbacks_.set_lobby_status_text) {
                                routing_callbacks_.set_lobby_status_text(message);
                              }
                              if (view_callbacks_.set_room_entry_status_text) {
                                view_callbacks_.set_room_entry_status_text(message);
                              }
                              return;
                            }

                            tempolink::juceapp::logging::Info(
                                "Media session started/joined successfully");

                            session_active_ = true;
                            active_room_code_ = room_code;
                            pending_room_code_.clear();
                            entry_room_code_.clear();

                            session_.SetMuted(false);
                            session_.SetInputGain(1.0F);
                            session_.SetInputReverb(0.0F);
                            session_.SetMetronomeEnabled(false);
                            session_.SetMetronomeBpm(
                                tempolink::juceapp::style::kDefaultMetronomeBpm);
                            session_.SetMetronomeTone(0);
                            session_.SetVolume(
                                tempolink::juceapp::style::kDefaultMasterVolume);

                            if (!status_context_.preferred_input_device.empty()) {
                              session_.SetInputDevice(status_context_.preferred_input_device);
                            }
                            if (!status_context_.preferred_output_device.empty()) {
                              session_.SetOutputDevice(status_context_.preferred_output_device);
                            }

                            if (view_callbacks_.set_session_room_title) {
                              view_callbacks_.set_session_room_title(
                                  "Jam Session  |  Room " + room_code);
                            }
                            if (view_callbacks_.set_session_mute) {
                              view_callbacks_.set_session_mute(false);
                            }
                            if (view_callbacks_.set_session_input_level) {
                              view_callbacks_.set_session_input_level(0.0F);
                            }
                            if (view_callbacks_.set_session_input_gain) {
                              view_callbacks_.set_session_input_gain(1.0F);
                            }
                            if (view_callbacks_.set_session_input_reverb) {
                              view_callbacks_.set_session_input_reverb(0.0F);
                            }
                            if (view_callbacks_.set_session_recording) {
                              view_callbacks_.set_session_recording(false);
                            }
                            if (view_callbacks_.set_session_audio_file_active) {
                              view_callbacks_.set_session_audio_file_active(false);
                            }
                            if (view_callbacks_.set_session_metronome_enabled) {
                              view_callbacks_.set_session_metronome_enabled(false);
                            }
                            if (view_callbacks_.set_session_metronome_bpm) {
                              view_callbacks_.set_session_metronome_bpm(
                                  tempolink::juceapp::style::kDefaultMetronomeBpm);
                            }
                            if (view_callbacks_.set_session_metronome_tone) {
                              view_callbacks_.set_session_metronome_tone(0);
                            }
                            if (view_callbacks_.set_session_master_volume) {
                              view_callbacks_.set_session_master_volume(
                                  tempolink::juceapp::style::kDefaultMasterVolume);
                            }

                            session_presence_controller_.reset();
                            session_presence_controller_.initializeParticipants(
                                room_snapshot.participants, status_context_.current_user_id,
                                status_context_.selected_part_label, session_.IsMuted());
                            RefreshSessionAudioDevices();
                            session_presence_controller_.refreshViewState(
                                session_active_, active_room_code_, tick_index,
                                signaling_client_.isConnected(), ice_config_loaded,
                                ice_config);

                            const bool signaling_ok = signaling_client_.connect(
                                status_context_.control_plane_host, status_context_.control_plane_port,
                                room_code, status_context_.current_user_id,
                                [this, alive](const SignalingClient::Event& event) {
                                  if (!alive->load()) {
                                    return;
                                  }
                                  session_presence_controller_.handleSignalingEvent(
                                      session_active_, active_room_code_,
                                      status_context_.current_user_id,
                                      status_context_.selected_part_label, event);
                                });
                            if (!signaling_ok) {
                              tempolink::juceapp::logging::Warn(
                                  "Signaling connect failed after media start: host=" +
                                  juce::String(status_context_.control_plane_host) +
                                  ", port=" +
                                  juce::String(status_context_.control_plane_port) +
                                  ", tls=" +
                                  juce::String(status_context_.control_plane_use_tls ? "true"
                                                                                      : "false"));
                              if (view_callbacks_.set_session_status_text) {
                                view_callbacks_.set_session_status_text(
                                    std::string(tempolink::juceapp::text::kMediaConnectedSignalingOffline));
                              }
                            } else {
                              tempolink::juceapp::logging::Info(
                                  "Signaling connect succeeded: host=" +
                                  juce::String(status_context_.control_plane_host) +
                                  ", port=" +
                                  juce::String(status_context_.control_plane_port) +
                                  ", tls=" +
                                  juce::String(status_context_.control_plane_use_tls ? "true"
                                                                                      : "false"));
                            }

                            if (routing_callbacks_.on_session_started) {
                              routing_callbacks_.on_session_started();
                            }
                            if (routing_callbacks_.switch_screen) {
                              routing_callbacks_.switch_screen(
                                  tempolink::juceapp::app::ScreenMode::Session);
                            }
                          });
}

void SessionLifecycleController::LeaveRoom() {
  tempolink::juceapp::logging::Info(
      "LeaveRoom requested: active=" +
      juce::String(session_active_ ? "true" : "false") +
      ", room=" + juce::String(active_room_code_));
  signaling_client_.disconnect();

  if (session_active_ && !active_room_code_.empty()) {
    room_api_.leaveRoomAsync(active_room_code_, status_context_.current_user_id,
                             [](bool, juce::String) {});
  }

  if (session_active_) {
    session_.Leave();
    session_.Stop();
  }

  session_active_ = false;
  active_room_code_.clear();
  pending_room_code_.clear();
  session_presence_controller_.reset();
  if (view_callbacks_.set_session_connection_state) {
    view_callbacks_.set_session_connection_state(false);
  }
  if (view_callbacks_.set_session_status_text) {
    view_callbacks_.set_session_status_text(
        std::string(tempolink::juceapp::text::kSessionDisconnected));
  }
  if (routing_callbacks_.on_session_ended) {
    routing_callbacks_.on_session_ended();
  }
}

void SessionLifecycleController::RefreshSessionAudioDevices() {
  session_presence_controller_.refreshAudioDevices(session_active_);
}

const std::string& SessionLifecycleController::active_room_code() const {
  return active_room_code_;
}

const std::string& SessionLifecycleController::entry_room_code() const {
  return entry_room_code_;
}

bool SessionLifecycleController::session_active() const { return session_active_; }

void SessionLifecycleController::clear_entry_room_code() { entry_room_code_.clear(); }
