#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

#include "tempolink/client/ClientSession.h"
#include "tempolink/juce/app/navigation/ScreenMode.h"
#include "tempolink/juce/app/RoomCatalog.h"
#include "tempolink/juce/app/session/SessionPresenceController.h"
#include "tempolink/juce/network/http/room/RoomApiClient.h"
#include "tempolink/juce/network/ice/IceConfigTypes.h"
#include "tempolink/juce/network/signaling/SignalingClient.h"
#include "tempolink/juce/ui/screens/LoginView.h"
#include "tempolink/juce/ui/screens/RoomEntrySettingsView.h"
#include "tempolink/juce/ui/screens/SessionView.h"

class SessionLifecycleController {
 public:
  SessionLifecycleController(
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
      std::string& preferred_input_device,
      std::string& preferred_output_device, const std::string& control_plane_host,
      const std::uint16_t& control_plane_port,
      const std::string& default_relay_host,
      const std::uint16_t& default_relay_port);

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
  LoginView& login_view_;
  RoomEntrySettingsView& room_entry_view_;
  SessionView& session_view_;
  std::shared_ptr<std::atomic_bool> alive_flag_;
  std::function<void(const juce::String&)> set_lobby_status_text_;
  std::function<void(tempolink::juceapp::app::ScreenMode)> switch_screen_;

  bool& auth_completed_;
  std::string& current_user_id_;
  std::string& current_display_name_;
  std::string& selected_part_label_;
  std::string& preferred_input_device_;
  std::string& preferred_output_device_;
  const std::string& control_plane_host_;
  const std::uint16_t& control_plane_port_;
  const std::string& default_relay_host_;
  const std::uint16_t& default_relay_port_;

  std::string active_room_code_;
  std::string pending_room_code_;
  std::string entry_room_code_;
  bool session_active_ = false;
};
