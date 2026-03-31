#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "tempolink/client/ClientSession.h"
#include "tempolink/juce/network/IceConfigClient.h"
#include "tempolink/juce/network/RoomApiClient.h"
#include "tempolink/juce/network/SignalingClient.h"
#include "tempolink/juce/ui/LobbyView.h"
#include "tempolink/juce/ui/RoomEntrySettingsView.h"
#include "tempolink/juce/ui/SessionView.h"

class ClientAppPresenter {
 public:
  ClientAppPresenter(LobbyView& lobby_view,
                     RoomEntrySettingsView& room_entry_view,
                     SessionView& session_view);
  ~ClientAppPresenter();

  void initialize();
  void shutdown();
  void tick();

 private:
  enum class ScreenMode { Lobby, RoomEntry, Session };

  void openRoomEntry(const std::string& room_code);
  void fetchRoomsFromApi();
  void fetchIceConfig();
  void openRoom(const std::string& room_code);
  void leaveRoom();
  void switchScreen(ScreenMode mode);

  void refreshSessionAudioDevices();
  void refreshSessionStatus();
  void refreshParticipantLevels();
  void setLobbyStatusText(const juce::String& base_message);
  void setParticipantsFromUserIds(const std::vector<std::string>& user_ids);
  void handleSignalingEvent(const SignalingClient::Event& event);
  void wireUiEvents();

  static std::uint32_t HashToU32(const std::string& text);
  static float AnimatedLevel(int tick_index, int phase_offset);
  static const RoomSummary* FindRoomByCode(const std::vector<RoomSummary>& rooms,
                                           const std::string& room_code);

  LobbyView& lobby_view_;
  RoomEntrySettingsView& room_entry_view_;
  SessionView& session_view_;

  ScreenMode screen_mode_ = ScreenMode::Lobby;
  std::string active_room_code_;
  std::string pending_room_code_;
  std::string entry_room_code_;
  std::string selected_part_label_ = "Guitar";
  std::string preferred_input_device_;
  std::string preferred_output_device_;
  std::string current_user_id_;
  int tick_index_ = 0;
  int lobby_refresh_tick_ = 0;
  bool room_fetch_in_flight_ = false;
  bool ice_fetch_in_flight_ = false;
  IceConfigSnapshot ice_config_;
  bool ice_config_loaded_ = false;

  std::shared_ptr<std::atomic_bool> alive_flag_ =
      std::make_shared<std::atomic_bool>(true);

  tempolink::client::ClientSession session_;
  bool session_active_ = false;
  IceConfigClient ice_client_{"http://127.0.0.1:8080"};
  RoomApiClient room_api_{"http://127.0.0.1:8080"};
  SignalingClient signaling_client_;

  std::vector<RoomSummary> rooms_;
  std::vector<ParticipantSummary> participants_;
};
