#include "tempolink/juce/app/ClientAppPresenter.h"

#include <utility>

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "tempolink/juce/config/ClientEnvConfig.h"
#include "tempolink/juce/constants/ClientText.h"
#include "tempolink/juce/bridge/AudioSessionService.h"

ClientAppPresenter::ClientAppPresenter(
    tempolink::juceapp::di::ViewRegistry views,
    tempolink::juceapp::di::DependencyContainer& deps)
    : views_(views),
      deps_(deps),
      status_context_(),
      screen_navigator_(views_.login_view, views_.lobby_view, views_.my_rooms_view,
                        views_.room_entry_view, views_.session_view, views_.profile_view,
                        views_.users_view, views_.news_view, views_.manual_view,
                        views_.qna_view, views_.settings_view),
      room_catalog_(),
      room_command_ui_coordinator_(
          tempolink::juceapp::app::ControllerCompositionFactory::CreateRoomCommandUiCoordinator(
              views_)),
      room_commands_(tempolink::juceapp::app::ControllerCompositionFactory::
                         CreateRoomCommandController(
                             deps_, room_catalog_, status_context_,
                             tempolink::juceapp::app::ClientAppCallbackFactory::
                                 CreateRoomCommandCallbacks(
                                     views_, room_command_ui_coordinator_,
                                     [this] { refreshRoomViews(); },
                                     [this] { fetchRoomsFromApi(); },
                                     [this](const std::string& text) {
                                       setLobbyStatusText(text);
                                     },
                                     [this](const std::string& room_code) {
                                       openRoomEntry(room_code);
                                     }))),
      auth_controller_(tempolink::juceapp::app::ControllerCompositionFactory::
                           CreateAuthController(deps_, views_, oauth_callback_host_,
                                                oauth_callback_port_,
                                                oauth_callback_path_)),
      lobby_data_controller_(
          tempolink::juceapp::app::ControllerCompositionFactory::CreateLobbyDataController(
              deps_, room_catalog_, views_, status_context_,
              [this](const std::string& text) { setLobbyStatusText(text); })),
      content_controller_(
          tempolink::juceapp::app::ControllerCompositionFactory::CreateContentController(
              deps_, views_, status_context_)),
      session_presence_controller_(tempolink::juceapp::app::ControllerCompositionFactory::
                                       CreateSessionPresenceController(deps_, views_)),
      session_lifecycle_callbacks_(
          tempolink::juceapp::app::ClientAppCallbackFactory::
              CreateSessionLifecycleCallbacks(
                  views_,
                  [this](const std::string& text) { setLobbyStatusText(text); },
                  [this](tempolink::juceapp::app::ScreenMode mode) {
                    screen_navigator_.SwitchTo(mode);
                  },
                  [this] {
                    session_recording_ = false;
                    session_audio_file_active_ = false;
                  },
                  [this] {
                    session_recording_ = false;
                    session_audio_file_active_ = false;
                  })),
      session_lifecycle_controller_(
          tempolink::juceapp::app::ControllerCompositionFactory::
              CreateSessionLifecycleController(
                  deps_, room_catalog_, status_context_, session_presence_controller_,
                  session_lifecycle_callbacks_)),
      auth_coordinator_(screen_navigator_, status_context_, lobby_data_controller_),
      lobby_coordinator_(screen_navigator_, status_context_, content_controller_),
      session_coordinator_(screen_navigator_, status_context_, session_lifecycle_controller_) {
  status_context_.selected_part_label = "Guitar";
  const auto env_config = tempolink::juceapp::config::ClientEnvConfig::Load();
  deps_.control_plane_base_url = env_config.control_plane_base_url;
  deps_.control_plane_host = env_config.control_plane_host;
  deps_.control_plane_port = env_config.control_plane_port;
  deps_.default_relay_host = env_config.default_relay_host;
  deps_.default_relay_port = env_config.default_relay_port;
  status_context_.control_plane_host = env_config.control_plane_host;
  status_context_.control_plane_port = env_config.control_plane_port;
  status_context_.default_relay_host = env_config.default_relay_host;
  status_context_.default_relay_port = env_config.default_relay_port;
  oauth_callback_host_ = env_config.oauth_callback_host;
  oauth_callback_port_ = env_config.oauth_callback_port;
  oauth_callback_path_ = env_config.oauth_callback_path;
  deps_.RebuildApiClients();
}

ClientAppPresenter::~ClientAppPresenter() { shutdown(); }

void ClientAppPresenter::initialize() {
  views_.session_view.setSignalingClient(deps_.signaling_client);
  wireLoginUiEvents();
  wireLobbyUiEvents();
  wireSessionUiEvents();
  wireUtilityUiEvents();
  wireUiEvents();
  refreshRoomViews();
  auth_controller_.Initialize(
      [this](const SocialAuthSession& session) { onAuthSessionReady(session); });
  auth_coordinator_.start();
}

void ClientAppPresenter::shutdown() {
  audio_file_chooser_.reset();
  audio_session_service_.reset();
  auth_controller_.Shutdown();
}

void ClientAppPresenter::tick() {
  auth_controller_.Tick();
  lobby_data_controller_.Tick(true);
  
  tick_index_++;
  
  if (session_lifecycle_controller_.session_active()) {
    deps_.session.Tick();
    session_presence_controller_.sendPeerPings(true);
    session_presence_controller_.refreshViewState(
        true, session_lifecycle_controller_.active_room_code(), tick_index_,
        deps_.signaling_client.isConnected(),
        lobby_data_controller_.ice_config_loaded(),
        lobby_data_controller_.ice_config());
  }

  if (audio_session_service_) {
    views_.session_view.setAudioFilePlaybackPosition(
        audio_session_service_->AudioFilePlaybackPosition());
    views_.session_view.setAudioFileLoopEnabled(
        audio_session_service_->IsAudioFileLoopEnabled());
  }
}

void ClientAppPresenter::setAudioBridge(
    std::shared_ptr<tempolink::client::AudioBridgePort> audio_bridge) {
  deps_.session.SetAudioBridge(std::move(audio_bridge));
}

void ClientAppPresenter::setAudioSessionService(
    std::shared_ptr<tempolink::juceapp::bridge::AudioSessionService>
        audio_session_service) {
  audio_session_service_ = std::move(audio_session_service);
  if (audio_session_service_) {
    deps_.session.SetAudioBridge(audio_session_service_->bridgePort());
    return;
  }
  deps_.session.SetAudioBridge(nullptr);
}

void ClientAppPresenter::refreshAuthProviders() {
  auth_controller_.RefreshAuthProviders();
}

void ClientAppPresenter::startOAuthLogin(const std::string& provider) {
  auth_controller_.StartOAuthLogin(provider);
}

void ClientAppPresenter::onAuthSessionReady(const SocialAuthSession& session) {
  auth_coordinator_.onAuthSuccess(session);

  views_.profile_view.setProfile(status_context_.current_display_name, "");
  views_.settings_view.setProfileFields(status_context_.current_display_name, "");

  setLobbyStatusText("Signed in as " + session.display_name.toStdString());
  content_controller_.refreshProfile();
}

void ClientAppPresenter::openRoomEntry(const std::string& room_code) {
  session_lifecycle_controller_.OpenRoomEntry(room_code);
}

void ClientAppPresenter::previewRoom(const std::string& room_code) {
  session_lifecycle_controller_.PreviewRoom(room_code);
}

void ClientAppPresenter::createRoomFromLobby(const std::string& host_user_id,
                                             const RoomCreatePayload& payload) {
  room_commands_.createRoom(host_user_id, payload);
}

void ClientAppPresenter::editRoomFromMyRooms(const std::string& room_code) {
  room_commands_.editRoom(room_code);
}

void ClientAppPresenter::deleteRoomFromMyRooms(const std::string& room_code) {
  room_commands_.deleteRoom(room_code);
}

void ClientAppPresenter::shareRoomFromMyRooms(const std::string& room_code) {
  room_commands_.shareRoom(room_code);
}

void ClientAppPresenter::updateLobbyFilter(const LobbyRoomFilter& filter) {
  lobby_data_controller_.UpdateFilter(filter);
}

void ClientAppPresenter::fetchRoomsFromApi() {
  lobby_data_controller_.FetchRooms();
}

void ClientAppPresenter::refreshRoomViews() {
  lobby_data_controller_.RefreshRoomViews();
}

void ClientAppPresenter::fetchIceConfig() {
  lobby_data_controller_.FetchIceConfig();
}

void ClientAppPresenter::openRoom(const std::string& room_code) {
  session_coordinator_.startSession(
      room_code, tick_index_, lobby_data_controller_.ice_config_loaded(),
      lobby_data_controller_.ice_config());
}

void ClientAppPresenter::leaveRoom() { session_coordinator_.leaveSession(); }

void ClientAppPresenter::refreshSessionAudioDevices() {
  session_lifecycle_controller_.RefreshSessionAudioDevices();
}

void ClientAppPresenter::openNavigationScreen(ILobbyView::NavigationTarget target) {
  lobby_coordinator_.navigate(target);
}

void ClientAppPresenter::setLobbyStatusText(const std::string& base_message) {
  std::string full_text = base_message;
  if (!status_context_.current_user_id.empty()) {
    full_text += " | User: " + status_context_.current_display_name;
  }
  views_.lobby_view.setStatusText(full_text);
  views_.my_rooms_view.setStatusText(full_text);
}

void ClientAppPresenter::wireUiEvents() {
  // Generic interactions
}
