#include "tempolink/juce/app/ClientAppPresenter.h"

#include <utility>

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "tempolink/juce/constants/ClientText.h"

ClientAppPresenter::ClientAppPresenter(LobbyView& lobby_view,
                                       LoginView& login_view,
                                       MyRoomsView& my_rooms_view,
                                       RoomEntrySettingsView& room_entry_view,
                                       SessionView& session_view,
                                       ProfileView& profile_view,
                                       UsersView& users_view,
                                       NewsView& news_view,
                                       ManualView& manual_view,
                                       QnaView& qna_view,
                                       SettingsView& settings_view)
    : lobby_view_(lobby_view),
      login_view_(login_view),
      my_rooms_view_(my_rooms_view),
      room_entry_view_(room_entry_view),
      session_view_(session_view),
      profile_view_(profile_view),
      users_view_(users_view),
      news_view_(news_view),
      manual_view_(manual_view),
      qna_view_(qna_view),
      settings_view_(settings_view),
      screen_navigator_(login_view_, lobby_view_, my_rooms_view_,
                        room_entry_view_, session_view_, profile_view_,
                        users_view_, news_view_, manual_view_, qna_view_,
                        settings_view_),
      room_commands_(room_api_, room_catalog_, my_rooms_view_, alive_flag_,
                     current_user_id_),
      auth_controller_(auth_api_, oauth_callback_server_, login_view_, alive_flag_,
                       oauth_callback_host_, oauth_callback_port_,
                       oauth_callback_path_),
      lobby_data_controller_(room_api_, ice_client_, room_catalog_, lobby_view_,
                             my_rooms_view_, alive_flag_, current_user_id_,
                             [this](const juce::String& text) {
                               setLobbyStatusText(text);
                             }),
      content_controller_(users_view_, news_view_, profile_view_, manual_view_,
                          qna_view_, settings_view_, users_api_, news_api_,
                          profile_api_, document_service_, session_,
                          alive_flag_, control_plane_base_url_,
                          current_user_id_, current_display_name_,
                          current_bio_),
      session_presence_controller_(session_view_, session_, signaling_client_),
      session_lifecycle_controller_(
          session_, room_api_, signaling_client_, session_presence_controller_,
          room_catalog_, login_view_, room_entry_view_, session_view_, alive_flag_,
          [this](const juce::String& text) { setLobbyStatusText(text); },
          [this](tempolink::juceapp::app::ScreenMode mode) {
            screen_navigator_.SwitchTo(mode);
          },
          auth_completed_, current_user_id_, current_display_name_,
          selected_part_label_, preferred_input_device_, preferred_output_device_,
          control_plane_host_, control_plane_port_, default_relay_host_,
          default_relay_port_),
      lobby_view_safe_(&lobby_view_) {
  const auto env_config = tempolink::juceapp::config::ClientEnvConfig::Load();
  control_plane_base_url_ = env_config.control_plane_base_url;
  control_plane_host_ = env_config.control_plane_host;
  control_plane_port_ = env_config.control_plane_port;
  default_relay_host_ = env_config.default_relay_host;
  default_relay_port_ = env_config.default_relay_port;
  oauth_callback_host_ = env_config.oauth_callback_host;
  oauth_callback_port_ = env_config.oauth_callback_port;
  oauth_callback_path_ = env_config.oauth_callback_path;

  ice_client_ = IceConfigClient(control_plane_base_url_);
  auth_api_ = AuthApiClient(control_plane_base_url_);
  room_api_ = RoomApiClient(control_plane_base_url_);
  users_api_ = UsersApiClient(control_plane_base_url_);
  news_api_ = NewsApiClient(control_plane_base_url_);
  profile_api_ = ProfileApiClient(control_plane_base_url_);
}

ClientAppPresenter::~ClientAppPresenter() { shutdown(); }

void ClientAppPresenter::initialize() {
  room_commands_.setCallbacks(
      [this] { refreshRoomViews(); }, [this] { fetchRoomsFromApi(); },
      [this](const juce::String& text) { setLobbyStatusText(text); },
      [this](const std::string& room_code) { openRoomEntry(room_code); });

  wireUiEvents();
  refreshRoomViews();
  auth_controller_.Initialize(
      [this](const SocialAuthSession& session) { onAuthSessionReady(session); });
  screen_navigator_.SwitchTo(tempolink::juceapp::app::ScreenMode::Login);
}

void ClientAppPresenter::shutdown() {
  if (!alive_flag_->load()) {
    return;
  }
  alive_flag_->store(false);
  session_lifecycle_controller_.LeaveRoom();
  auth_controller_.Shutdown();
}

void ClientAppPresenter::tick() {
  ++tick_index_;
  auth_controller_.Tick();

  const auto mode = screen_navigator_.CurrentMode();
  const bool in_lobby_mode =
      mode == tempolink::juceapp::app::ScreenMode::Lobby ||
      mode == tempolink::juceapp::app::ScreenMode::MyRooms;
  lobby_data_controller_.Tick(in_lobby_mode);

  if (!session_lifecycle_controller_.session_active()) {
    return;
  }

  session_.Tick();
  session_presence_controller_.sendPeerPings(true);
  session_presence_controller_.refreshViewState(
      true, session_lifecycle_controller_.active_room_code(), tick_index_,
      signaling_client_.isConnected(), lobby_data_controller_.ice_config_loaded(),
      lobby_data_controller_.ice_config());
}

void ClientAppPresenter::setAudioBridge(
    std::shared_ptr<tempolink::client::AudioBridgePort> audio_bridge) {
  session_.SetAudioBridge(std::move(audio_bridge));
}

void ClientAppPresenter::fetchRoomsFromApi() { lobby_data_controller_.FetchRooms(); }

void ClientAppPresenter::refreshRoomViews() {
  lobby_data_controller_.RefreshRoomViews();
}

void ClientAppPresenter::fetchIceConfig() { lobby_data_controller_.FetchIceConfig(); }

void ClientAppPresenter::refreshAuthProviders() {
  auth_controller_.RefreshAuthProviders();
}

void ClientAppPresenter::startOAuthLogin(const std::string& provider) {
  auth_controller_.StartOAuthLogin(provider);
}

void ClientAppPresenter::onAuthSessionReady(const SocialAuthSession& session) {
  auth_completed_ = true;
  if (session.user_id.isNotEmpty()) {
    current_user_id_ = session.user_id.toStdString();
  } else if (session.provider_user_id.isNotEmpty()) {
    current_user_id_ = (session.provider + "-" + session.provider_user_id).toStdString();
  } else {
    const int suffix = 100000 + juce::Random::getSystemRandom().nextInt(900000);
    current_user_id_ = "user-" + std::to_string(suffix);
  }
  current_display_name_ = session.display_name.isNotEmpty()
                              ? session.display_name.toStdString()
                              : current_user_id_;
  current_bio_.clear();
  profile_view_.setProfile(current_display_name_, current_bio_);
  settings_view_.setProfileFields(current_display_name_, current_bio_);

  setLobbyStatusText("Signed in as " + session.display_name);
  content_controller_.refreshProfile();
  fetchIceConfig();
  fetchRoomsFromApi();
  screen_navigator_.SwitchTo(tempolink::juceapp::app::ScreenMode::Lobby);
}

void ClientAppPresenter::openRoomEntry(const std::string& room_code) {
  session_lifecycle_controller_.OpenRoomEntry(room_code);
}

void ClientAppPresenter::previewRoom(const std::string& room_code) {
  session_lifecycle_controller_.PreviewRoom(room_code);
}

void ClientAppPresenter::createRoomFromLobby(const std::string& host_user_id,
                                             int max_participants) {
  room_commands_.createRoom(host_user_id, max_participants);
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

void ClientAppPresenter::openRoom(const std::string& room_code) {
  session_lifecycle_controller_.OpenRoom(
      room_code, tick_index_, lobby_data_controller_.ice_config_loaded(),
      lobby_data_controller_.ice_config());
}

void ClientAppPresenter::leaveRoom() { session_lifecycle_controller_.LeaveRoom(); }

void ClientAppPresenter::refreshSessionAudioDevices() {
  session_lifecycle_controller_.RefreshSessionAudioDevices();
}

void ClientAppPresenter::openNavigationScreen(LobbyView::NavigationTarget target) {
  using Nav = LobbyView::NavigationTarget;
  using ScreenMode = tempolink::juceapp::app::ScreenMode;

  switch (target) {
    case Nav::Rooms:
      screen_navigator_.SwitchTo(ScreenMode::Lobby);
      return;
    case Nav::Profile:
      content_controller_.refreshProfile();
      screen_navigator_.SwitchTo(ScreenMode::Profile);
      break;
    case Nav::Users:
      content_controller_.refreshUsers();
      screen_navigator_.SwitchTo(ScreenMode::Users);
      break;
    case Nav::News:
      content_controller_.refreshNews();
      screen_navigator_.SwitchTo(ScreenMode::News);
      break;
    case Nav::Manual:
      content_controller_.refreshManual();
      screen_navigator_.SwitchTo(ScreenMode::Manual);
      break;
    case Nav::Qna:
      content_controller_.refreshQna();
      screen_navigator_.SwitchTo(ScreenMode::Qna);
      break;
    case Nav::Settings:
      content_controller_.refreshProfile();
      content_controller_.refreshSettingsView();
      screen_navigator_.SwitchTo(ScreenMode::Settings);
      break;
    default:
      screen_navigator_.SwitchTo(ScreenMode::Lobby);
      return;
  }
}

void ClientAppPresenter::setLobbyStatusText(const juce::String& base_message) {
  if (!alive_flag_->load()) {
    return;
  }

  juce::String message = base_message;
  if (lobby_data_controller_.ice_config_loaded()) {
    message += " | net: ";
    message += (lobby_data_controller_.ice_config().hasDirectAssist()
                    ? "p2p-assist"
                    : "limited-direct");
  }

  auto lobby_safe = lobby_view_safe_;
  auto apply_status = [lobby_safe, message]() mutable {
    if (auto* lobby = lobby_safe.getComponent(); lobby != nullptr) {
      lobby->setStatusText(message);
    }
  };

  auto* message_manager = juce::MessageManager::getInstanceWithoutCreating();
  if (message_manager == nullptr) {
    return;
  }

  if (message_manager->isThisTheMessageThread()) {
    apply_status();
    return;
  }

  auto alive = alive_flag_;
  juce::MessageManager::callAsync([alive, apply_status = std::move(apply_status)]() mutable {
    if (!alive->load()) {
      return;
    }
    apply_status();
  });
}

void ClientAppPresenter::wireUiEvents() {
  wireLoginUiEvents();
  wireLobbyUiEvents();
  wireSessionUiEvents();
  wireUtilityUiEvents();
}
