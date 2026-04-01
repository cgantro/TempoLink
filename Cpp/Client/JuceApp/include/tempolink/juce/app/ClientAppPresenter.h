#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "tempolink/client/ClientSession.h"
#include "tempolink/config/NetworkConstants.h"
#include "tempolink/juce/config/ClientEnvConfig.h"
#include "tempolink/juce/network/ice/IceConfigClient.h"
#include "tempolink/juce/network/auth/AuthApiClient.h"
#include "tempolink/juce/network/http/news/NewsApiClient.h"
#include "tempolink/juce/network/auth/OAuthCallbackServer.h"
#include "tempolink/juce/network/http/room/RoomApiClient.h"
#include "tempolink/juce/network/signaling/SignalingClient.h"
#include "tempolink/juce/network/http/user/ProfileApiClient.h"
#include "tempolink/juce/network/http/user/UsersApiClient.h"
#include "tempolink/juce/network/http/common/DocumentContentService.h"
#include "tempolink/juce/constants/AuthConstants.h"
#include "tempolink/juce/app/RoomCatalog.h"
#include "tempolink/juce/app/RoomCommandController.h"
#include "tempolink/juce/ui/screens/LobbyView.h"
#include "tempolink/juce/ui/screens/LoginView.h"
#include "tempolink/juce/ui/screens/ManualView.h"
#include "tempolink/juce/ui/screens/MyRoomsView.h"
#include "tempolink/juce/ui/screens/NewsView.h"
#include "tempolink/juce/ui/screens/ProfileView.h"
#include "tempolink/juce/ui/screens/QnaView.h"
#include "tempolink/juce/ui/screens/RoomEntrySettingsView.h"
#include "tempolink/juce/ui/screens/SessionView.h"
#include "tempolink/juce/ui/screens/SettingsView.h"
#include "tempolink/juce/ui/screens/UsersView.h"

class ClientAppPresenter {
 public:
  ClientAppPresenter(LobbyView& lobby_view,
                     LoginView& login_view,
                     MyRoomsView& my_rooms_view,
                     RoomEntrySettingsView& room_entry_view,
                     SessionView& session_view,
                     ProfileView& profile_view,
                     UsersView& users_view,
                     NewsView& news_view,
                     ManualView& manual_view,
                     QnaView& qna_view,
                     SettingsView& settings_view);
  ~ClientAppPresenter();

  void initialize();
  void shutdown();
  void tick();
  void setAudioBridge(std::shared_ptr<tempolink::client::AudioBridgePort> audio_bridge);

 private:
  enum class ScreenMode {
    Login,
    Lobby,
    MyRooms,
    RoomEntry,
    Session,
    Profile,
    Users,
    News,
    Manual,
    Qna,
    Settings
  };

  void openRoomEntry(const std::string& room_code);
  void refreshAuthProviders();
  void startOAuthLogin(const std::string& provider);
  void handleOAuthCallback(const std::string& ticket,
                           const std::string& provider,
                           const std::string& error,
                           const std::string& message);
  void onAuthSessionReady(const SocialAuthSession& session);
  void previewRoom(const std::string& room_code);
  void createRoomFromLobby(const std::string& host_user_id, int max_participants);
  void editRoomFromMyRooms(const std::string& room_code);
  void deleteRoomFromMyRooms(const std::string& room_code);
  void shareRoomFromMyRooms(const std::string& room_code);
  void updateLobbyFilter(const LobbyRoomFilter& filter);
  void fetchRoomsFromApi();
  void refreshRoomViews();
  void fetchIceConfig();
  void openRoom(const std::string& room_code);
  void leaveRoom();
  void switchScreen(ScreenMode mode);

  void refreshSessionAudioDevices();
  void refreshSessionViewState();
  void openNavigationScreen(LobbyView::NavigationTarget target);
  void refreshUsers(const std::string& query = {});
  void refreshNews();
  void refreshManual();
  void refreshQna();
  void refreshProfile();
  void applySettingsAudioFormat(int sample_rate_hz, int buffer_samples);
  void saveProfileSettings(const std::string& display_name, const std::string& bio);
  void refreshSettingsView();
  void sendPeerPings();
  void setLobbyStatusText(const juce::String& base_message);
  void rebuildParticipantsFromUserIds(const std::vector<std::string>& user_ids);
  void handleSignalingEvent(const SignalingClient::Event& event);
  void wireUiEvents();

  LobbyView& lobby_view_;
  LoginView& login_view_;
  MyRoomsView& my_rooms_view_;
  RoomEntrySettingsView& room_entry_view_;
  SessionView& session_view_;
  ProfileView& profile_view_;
  UsersView& users_view_;
  NewsView& news_view_;
  ManualView& manual_view_;
  QnaView& qna_view_;
  SettingsView& settings_view_;

  ScreenMode screen_mode_ = ScreenMode::Login;
  std::string active_room_code_;
  std::string pending_room_code_;
  std::string entry_room_code_;
  std::string selected_part_label_ = "Guitar";
  std::string preferred_input_device_;
  std::string preferred_output_device_;
  std::string current_user_id_;
  std::string current_display_name_;
  std::string current_bio_;
  bool auth_completed_ = false;
  std::string oauth_redirect_uri_;
  bool oauth_login_pending_ = false;
  std::uint32_t oauth_login_started_ms_ = 0;
  std::string oauth_pending_provider_;
  std::string control_plane_base_url_ = tempolink::config::kDefaultControlPlaneBaseUrl;
  std::string control_plane_host_ = tempolink::config::kDefaultControlPlaneHost;
  std::uint16_t control_plane_port_ = tempolink::config::kDefaultControlPlanePort;
  std::string default_relay_host_ = tempolink::config::kDefaultRelayHost;
  std::uint16_t default_relay_port_ = tempolink::config::kDefaultRelayPort;
  std::string oauth_callback_host_ = tempolink::juceapp::constants::kOAuthCallbackHost;
  std::uint16_t oauth_callback_port_ = tempolink::juceapp::constants::kOAuthCallbackPort;
  std::string oauth_callback_path_ = tempolink::juceapp::constants::kOAuthCallbackPath;
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
  IceConfigClient ice_client_{tempolink::config::kDefaultControlPlaneBaseUrl};
  AuthApiClient auth_api_{tempolink::config::kDefaultControlPlaneBaseUrl};
  OAuthCallbackServer oauth_callback_server_;
  RoomApiClient room_api_{tempolink::config::kDefaultControlPlaneBaseUrl};
  UsersApiClient users_api_{tempolink::config::kDefaultControlPlaneBaseUrl};
  NewsApiClient news_api_{tempolink::config::kDefaultControlPlaneBaseUrl};
  ProfileApiClient profile_api_{tempolink::config::kDefaultControlPlaneBaseUrl};
  DocumentContentService document_service_;
  SignalingClient signaling_client_;
  RoomCatalog room_catalog_;
  RoomCommandController room_commands_;
  std::vector<ParticipantSummary> participants_;
  LobbyRoomFilter lobby_filter_;
  std::unordered_map<std::string, int> peer_latency_ms_;
  std::unordered_map<std::string, std::uint64_t> peer_ping_last_sent_ms_;
  std::chrono::steady_clock::time_point last_peer_ping_tick_ =
      std::chrono::steady_clock::time_point::min();
  juce::Component::SafePointer<LobbyView> lobby_view_safe_;
};
