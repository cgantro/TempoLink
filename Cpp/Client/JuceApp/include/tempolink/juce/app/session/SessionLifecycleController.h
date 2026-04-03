#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "tempolink/client/ClientSession.h"
#include "tempolink/juce/app/navigation/ScreenMode.h"
#include "tempolink/juce/app/RoomCatalog.h"
#include "tempolink/juce/app/session/SessionPresenceController.h"
#include "tempolink/juce/network/http/room/RoomApiClient.h"
#include "tempolink/juce/network/ice/IceConfigTypes.h"
#include "tempolink/juce/network/signaling/SignalingClient.h"
#include "tempolink/juce/app/AppStatusContext.h"

using AppStatusContext = tempolink::juceapp::app::AppStatusContext;

class SessionLifecycleController {
 public:
  struct SessionServices {
    tempolink::client::ClientSession& session;
    RoomApiClient& room_api;
    SignalingClient& signaling_client;
    SessionPresenceController& session_presence_controller;
    RoomCatalog& room_catalog;
  };

  struct SessionViewCallbacks {
    std::function<void(const std::string&)> set_login_status_text;
    std::function<void(const RoomSummary&)> set_room_entry_room;
    std::function<void(const std::string&)> set_room_entry_selected_part;
    std::function<void(const std::vector<std::string>&, const std::string&)>
        set_room_entry_input_devices;
    std::function<void(const std::vector<std::string>&, const std::string&)>
        set_room_entry_output_devices;
    std::function<void(const std::string&)> set_room_entry_status_text;
    std::function<void(const std::string&)> set_session_room_title;
    std::function<void(bool)> set_session_connection_state;
    std::function<void(const std::string&)> set_session_status_text;
    std::function<void(bool)> set_session_mute;
    std::function<void(float)> set_session_input_level;
    std::function<void(float)> set_session_input_gain;
    std::function<void(float)> set_session_input_reverb;
    std::function<void(bool)> set_session_recording;
    std::function<void(bool)> set_session_audio_file_active;
    std::function<void(bool)> set_session_metronome_enabled;
    std::function<void(int)> set_session_metronome_bpm;
    std::function<void(int)> set_session_metronome_tone;
    std::function<void(float)> set_session_master_volume;
  };

  struct SessionRoutingCallbacks {
    std::function<void(const std::string&)> set_lobby_status_text;
    std::function<void(tempolink::juceapp::app::ScreenMode)> switch_screen;
    std::function<void()> on_session_started;
    std::function<void()> on_session_ended;
  };

  SessionLifecycleController(SessionServices services, AppStatusContext& status_context,
                             SessionViewCallbacks view_callbacks,
                             SessionRoutingCallbacks routing_callbacks,
                             std::shared_ptr<std::atomic_bool> alive_flag);

  void OpenRoomEntry(const std::string& room_code);
  void PreviewRoom(const std::string& room_code);
  void OpenRoom(const std::string& room_code, int tick_index,
                bool ice_config_loaded, const IceConfigSnapshot& ice_config);
  void LeaveRoom();
  void RefreshSessionAudioDevices();

  const std::string& active_room_code() const;
  const std::string& entry_room_code() const;
  bool session_active() const;
  void clear_entry_room_code();

 private:
  tempolink::client::ClientSession& session_;
  RoomApiClient& room_api_;
  SignalingClient& signaling_client_;
  SessionPresenceController& session_presence_controller_;
  RoomCatalog& room_catalog_;
  std::shared_ptr<std::atomic_bool> alive_flag_;
  SessionViewCallbacks view_callbacks_;
  SessionRoutingCallbacks routing_callbacks_;

  AppStatusContext& status_context_;

  std::string active_room_code_;
  std::string pending_room_code_;
  std::string entry_room_code_;
  bool session_active_ = false;
};
