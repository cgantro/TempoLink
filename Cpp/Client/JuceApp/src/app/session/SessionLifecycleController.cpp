#include "tempolink/juce/app/session/SessionLifecycleController.h"

#include <utility>

#include "tempolink/juce/app/SessionModelSupport.h"
#include "tempolink/juce/constants/ClientText.h"
#include "tempolink/juce/logging/AppLogger.h"
#include "tempolink/juce/style/UiStyle.h"

SessionLifecycleController::SessionLifecycleController(
    tempolink::client::ClientSession& session, RoomApiClient& room_api,
    SignalingClient& signaling_client,
    SessionPresenceController& session_presence_controller,
    RoomCatalog& room_catalog, LoginView& login_view,
    RoomEntrySettingsView& room_entry_view, SessionView& session_view,
    std::shared_ptr<std::atomic_bool> alive_flag,
    const std::function<void(const juce::String&)>& set_lobby_status_text,
    const std::function<void(tempolink::juceapp::app::ScreenMode)>&
        switch_screen,
    bool& auth_completed, std::string& current_user_id,
    std::string& current_display_name, std::string& selected_part_label,
    std::string& preferred_input_device, std::string& preferred_output_device,
    const std::string& control_plane_host,
    const std::uint16_t& control_plane_port,
    const std::string& default_relay_host,
    const std::uint16_t& default_relay_port)
    : session_(session),
      room_api_(room_api),
      signaling_client_(signaling_client),
      session_presence_controller_(session_presence_controller),
      room_catalog_(room_catalog),
      login_view_(login_view),
      room_entry_view_(room_entry_view),
      session_view_(session_view),
      alive_flag_(std::move(alive_flag)),
      set_lobby_status_text_(set_lobby_status_text),
      switch_screen_(switch_screen),
      auth_completed_(auth_completed),
      current_user_id_(current_user_id),
      current_display_name_(current_display_name),
      selected_part_label_(selected_part_label),
      preferred_input_device_(preferred_input_device),
      preferred_output_device_(preferred_output_device),
      control_plane_host_(control_plane_host),
      control_plane_port_(control_plane_port),
      default_relay_host_(default_relay_host),
      default_relay_port_(default_relay_port) {}

void SessionLifecycleController::OpenRoomEntry(const std::string& room_code) {
  const RoomSummary* room = room_catalog_.findByCode(room_code);
  if (room == nullptr) {
    set_lobby_status_text_(tempolink::juceapp::text::kRoomNotFound);
    switch_screen_(tempolink::juceapp::app::ScreenMode::Lobby);
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
  if (inputs.empty() || outputs.empty()) {
    room_entry_view_.setStatusText(
        "오디오 장치 없음: 입력/출력 장치를 연결한 뒤 다시 시도하세요.");
  } else {
    room_entry_view_.setStatusText(tempolink::juceapp::text::kEntryPreflightGuide);
  }
  switch_screen_(tempolink::juceapp::app::ScreenMode::RoomEntry);
}

void SessionLifecycleController::PreviewRoom(const std::string& room_code) {
  OpenRoomEntry(room_code);
  room_entry_view_.setStatusText(
      "PREVIEW selected. Monitor-only mode will be enabled in next step.");
}

void SessionLifecycleController::OpenRoom(const std::string& room_code,
                                          int tick_index, bool ice_config_loaded,
                                          const IceConfigSnapshot& ice_config) {
  if (!auth_completed_ || current_user_id_.empty()) {
    login_view_.setStatusText("Please sign in before joining rooms.");
    switch_screen_(tempolink::juceapp::app::ScreenMode::Login);
    return;
  }

  LeaveRoom();

  const RoomSummary* room = room_catalog_.findByCode(room_code);
  if (room == nullptr) {
    set_lobby_status_text_(tempolink::juceapp::text::kRoomNotFound);
    return;
  }

  pending_room_code_ = room_code;
  const juce::String joining_text =
      juce::String(tempolink::juceapp::text::kJoiningRoomPrefix) +
      juce::String(room_code) + tempolink::juceapp::text::kJoinPendingSuffix;
  set_lobby_status_text_(joining_text);
  room_entry_view_.setStatusText(joining_text);

  auto alive = alive_flag_;
  room_api_.joinRoomAsync(room_code, current_user_id_,
                          [this, alive, room_code, tick_index, ice_config_loaded,
                           ice_config](bool ok, juce::String error_text) {
                            if (!alive->load()) {
                              return;
                            }
                            if (pending_room_code_ != room_code) {
                              return;
                            }
                            if (!ok) {
                              pending_room_code_.clear();
                              const juce::String message =
                                  juce::String(
                                      tempolink::juceapp::text::kJoinFailedPrefix) +
                                  error_text;
                              set_lobby_status_text_(message);
                              room_entry_view_.setStatusText(message);
                              return;
                            }

                            const RoomSummary* room_after_join =
                                room_catalog_.findByCode(room_code);
                            if (room_after_join == nullptr) {
                              tempolink::juceapp::logging::Error(
                                  "Join room succeeded but room metadata missing");
                              pending_room_code_.clear();
                              const juce::String message =
                                  tempolink::juceapp::text::kJoinMetadataMissing;
                              set_lobby_status_text_(message);
                              room_entry_view_.setStatusText(message);
                              return;
                            }

                            tempolink::client::ClientSession::Config config;
                            config.server_host = room_after_join->relay_host.empty()
                                                     ? default_relay_host_
                                                     : room_after_join->relay_host;
                            config.server_port = room_after_join->relay_port == 0
                                                     ? default_relay_port_
                                                     : room_after_join->relay_port;
                            config.room_id =
                                tempolink::juceapp::app::HashToU32(room_code);
                            config.participant_id =
                                tempolink::juceapp::app::HashToU32(current_user_id_);
                            config.nickname =
                                current_display_name_.empty() ? current_user_id_
                                                              : current_display_name_;

                            if (!session_.Start(config) || !session_.Join()) {
                              tempolink::juceapp::logging::Error(
                                  "Media session start/join failed roomCode=" +
                                  juce::String(room_code));
                              session_.Stop();
                              pending_room_code_.clear();
                              const juce::String message =
                                  tempolink::juceapp::text::kStartMediaSessionFailed;
                              set_lobby_status_text_(message);
                              room_entry_view_.setStatusText(message);
                              return;
                            }

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
                            session_view_.setInputLevel(0.0F);
                            session_view_.setInputGain(1.0F);
                            session_view_.setInputReverb(0.0F);
                            session_view_.setRecording(false);
                            session_view_.setAudioFileActive(false);
                            session_view_.setMetronomeEnabled(false);
                            session_view_.setMetronomeBpm(
                                tempolink::juceapp::style::kDefaultMetronomeBpm);
                            session_view_.setMasterVolume(
                                tempolink::juceapp::style::kDefaultMasterVolume);

                            session_presence_controller_.reset();
                            session_presence_controller_.initializeParticipants(
                                room_after_join->participants, current_user_id_,
                                selected_part_label_, session_.IsMuted());
                            RefreshSessionAudioDevices();
                            session_presence_controller_.refreshViewState(
                                session_active_, active_room_code_, tick_index,
                                signaling_client_.isConnected(), ice_config_loaded,
                                ice_config);

                            const bool signaling_ok = signaling_client_.connect(
                                control_plane_host_, control_plane_port_, room_code,
                                current_user_id_,
                                [this, alive](const SignalingClient::Event& event) {
                                  if (!alive->load()) {
                                    return;
                                  }
                                  session_presence_controller_.handleSignalingEvent(
                                      session_active_, active_room_code_,
                                      current_user_id_, selected_part_label_,
                                      event);
                                });
                            if (!signaling_ok) {
                              session_view_.setStatusText(
                                  tempolink::juceapp::text::
                                      kMediaConnectedSignalingOffline);
                            }

                            switch_screen_(
                                tempolink::juceapp::app::ScreenMode::Session);
                          });
}

void SessionLifecycleController::LeaveRoom() {
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
  session_presence_controller_.reset();
  session_view_.setConnectionState(false);
  session_view_.setStatusText(tempolink::juceapp::text::kSessionDisconnected);
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
