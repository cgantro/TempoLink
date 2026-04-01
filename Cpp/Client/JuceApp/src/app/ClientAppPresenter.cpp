#include "tempolink/juce/app/ClientAppPresenter.h"

#include <algorithm>
#include <vector>
#include <utility>

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "tempolink/juce/app/LobbyBindings.h"
#include "tempolink/juce/app/ParticipantRosterBuilder.h"
#include "tempolink/juce/app/SessionModelSupport.h"
#include "tempolink/juce/app/SessionBindings.h"
#include "tempolink/juce/constants/ApiPaths.h"
#include "tempolink/juce/constants/AuthConstants.h"
#include "tempolink/juce/constants/ClientText.h"
#include "tempolink/juce/logging/AppLogger.h"
#include "tempolink/juce/style/UiStyle.h"
#include "tempolink/config/NetworkConstants.h"

namespace {
constexpr std::uint32_t kOAuthLoginTimeoutMs = 90U * 1000U;

std::uint64_t NowSteadyMs() {
  return static_cast<std::uint64_t>(
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now().time_since_epoch())
          .count());
}
}

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
      room_commands_(room_api_, room_catalog_, my_rooms_view_, alive_flag_,
                     current_user_id_),
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

  const bool callback_started = oauth_callback_server_.start(
      oauth_callback_port_,
      [this, alive = alive_flag_](const std::string& ticket, const std::string& provider,
             const std::string& error, const std::string& message) {
        if (!alive->load()) {
          return;
        }
        handleOAuthCallback(ticket, provider, error, message);
      });
  if (callback_started) {
    oauth_redirect_uri_ = std::string("http://") + oauth_callback_host_ + ":" +
                          std::to_string(oauth_callback_server_.port()) +
                          oauth_callback_path_;
  } else {
    oauth_redirect_uri_.clear();
  }

  login_view_.setStatusText(callback_started
                                ? "Continue with Google login."
                                : "OAuth callback server failed to start.");
  refreshAuthProviders();
  switchScreen(ScreenMode::Login);
}

void ClientAppPresenter::shutdown() {
  if (!alive_flag_->load()) {
    return;
  }
  alive_flag_->store(false);
  leaveRoom();
  oauth_callback_server_.stop();
}

void ClientAppPresenter::tick() {
  ++tick_index_;

  if (screen_mode_ == ScreenMode::Login && oauth_login_pending_) {
    const auto now_ms = juce::Time::getMillisecondCounter();
    if (now_ms - oauth_login_started_ms_ >= kOAuthLoginTimeoutMs) {
      oauth_login_pending_ = false;
      oauth_pending_provider_.clear();
      login_view_.setBusy(false);
      login_view_.setStatusText(
          "Login cancelled or timed out. Click Google login to try again.");
    }
  }

  if (screen_mode_ == ScreenMode::Lobby || screen_mode_ == ScreenMode::MyRooms) {
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
  sendPeerPings();
  refreshSessionViewState();
}

void ClientAppPresenter::setAudioBridge(
    std::shared_ptr<tempolink::client::AudioBridgePort> audio_bridge) {
  session_.SetAudioBridge(std::move(audio_bridge));
}

void ClientAppPresenter::fetchRoomsFromApi() {
  if (room_fetch_in_flight_) {
    return;
  }

  room_fetch_in_flight_ = true;
  RoomListFilter api_filter;
  api_filter.query = lobby_filter_.query;
  api_filter.tag = lobby_filter_.tag;
  api_filter.is_public = lobby_filter_.is_public;
  api_filter.has_password = lobby_filter_.has_password;
  api_filter.mode = lobby_filter_.mode;
  auto alive = alive_flag_;
  room_api_.fetchRoomsAsync(
      api_filter,
      [this, alive](bool ok, std::vector<RoomSummary> rooms, juce::String error_text) {
        if (!alive->load()) {
          return;
        }
        room_fetch_in_flight_ = false;
        if (!ok) {
          room_catalog_.clear();
          refreshRoomViews();
          setLobbyStatusText(
              juce::String(tempolink::juceapp::text::kRoomApiUnavailablePrefix) +
              error_text + tempolink::juceapp::text::kShowingMockRoomsSuffix);
          my_rooms_view_.setStatusText("Failed to load MY ROOMS: " + error_text);
          return;
        }

        if (rooms.empty()) {
          room_catalog_.clear();
          refreshRoomViews();
          setLobbyStatusText(tempolink::juceapp::text::kRoomApiEmptyFallback);
          my_rooms_view_.setStatusText("No rooms created yet.");
          return;
        }

        room_catalog_.setRooms(std::move(rooms));
        const int room_count =
            static_cast<int>(room_catalog_.allRooms().size());
        refreshRoomViews();
        setLobbyStatusText(juce::String(tempolink::juceapp::text::kLoadedRoomsPrefix) +
                           juce::String(room_count) +
                           tempolink::juceapp::text::kLoadedRoomsUserDelimiter +
                           juce::String(current_user_id_));
        my_rooms_view_.setStatusText("My rooms: " +
                                     juce::String(static_cast<int>(
                                         room_catalog_.ownedBy(current_user_id_).size())));
      });
}

void ClientAppPresenter::refreshRoomViews() {
  lobby_view_.setRooms(room_catalog_.allRooms());
  my_rooms_view_.setRooms(room_catalog_.ownedBy(current_user_id_));
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
          setLobbyStatusText(
              juce::String(tempolink::juceapp::text::kIceConfigUnavailablePrefix) +
              error_text);
          return;
        }

        ice_config_ = std::move(snapshot);
        ice_config_loaded_ = true;
        const juce::String direct_hint = ice_config_.hasDirectAssist()
                                             ? tempolink::juceapp::text::kP2PAssistReady
                                             : tempolink::juceapp::text::kP2PAssistLimited;
        setLobbyStatusText(
            juce::String(tempolink::juceapp::text::kNetworkProfileLoadedPrefix) +
            direct_hint +
            tempolink::juceapp::text::kNetworkProfileLoadedSuffix);
      });
}

void ClientAppPresenter::refreshAuthProviders() {
  login_view_.setBusy(true);
  login_view_.setStatusText("Loading OAuth providers...");
  auto alive = alive_flag_;
  auth_api_.fetchProvidersAsync(
      [this, alive](bool ok, std::vector<AuthProviderInfo> providers,
                    juce::String error_text) {
        if (!alive->load()) {
          return;
        }
        login_view_.setBusy(false);
        if (!ok) {
          login_view_.setStatusText("Failed to load providers: " + error_text);
          login_view_.setProviders({});
          return;
        }

        login_view_.setProviders(providers);
        const bool has_enabled = std::any_of(
            providers.begin(), providers.end(),
            [](const AuthProviderInfo& provider) { return provider.enabled; });
        if (has_enabled) {
          login_view_.setStatusText("Providers loaded. Continue with social login.");
        } else {
          login_view_.setStatusText("No OAuth provider configured on server.");
        }
      });
}

void ClientAppPresenter::startOAuthLogin(const std::string& provider) {
  if (oauth_redirect_uri_.empty()) {
    login_view_.setBusy(false);
    login_view_.setStatusText("OAuth callback server is unavailable.");
    return;
  }

  login_view_.setStatusText("Opening browser for " + juce::String(provider) + " login...");
  const auto start_url = auth_api_.buildStartUrl(provider, oauth_redirect_uri_);
  tempolink::juceapp::logging::Info(
      "OAuth start provider=" + juce::String(provider) +
      " redirectUri=" + juce::String(oauth_redirect_uri_) +
      " startUrl=" + juce::String(start_url));
  const bool launched = juce::URL(start_url).launchInDefaultBrowser();
  if (!launched) {
    oauth_login_pending_ = false;
    oauth_pending_provider_.clear();
    login_view_.setBusy(false);
    login_view_.setStatusText("Failed to launch browser.");
    return;
  }

  oauth_login_pending_ = true;
  oauth_login_started_ms_ = juce::Time::getMillisecondCounter();
  oauth_pending_provider_ = provider;

  // Keep login controls enabled so user can retry immediately if browser is closed.
  login_view_.setBusy(false);
  login_view_.setStatusText(
      "Browser opened. Complete Google login, or click login again if you closed it.");
}

void ClientAppPresenter::handleOAuthCallback(const std::string& ticket,
                                             const std::string& provider,
                                             const std::string& error,
                                             const std::string& message) {
  if (auth_completed_) {
    return;
  }

  oauth_login_pending_ = false;
  oauth_pending_provider_.clear();

  if (!error.empty()) {
    login_view_.setBusy(false);
    login_view_.setStatusText("Login failed (" + juce::String(provider) + "): " +
                              juce::String(message));
    return;
  }

  if (ticket.empty()) {
    login_view_.setBusy(false);
    login_view_.setStatusText("OAuth callback missing ticket.");
    return;
  }

  login_view_.setBusy(true);
  login_view_.setStatusText("Exchanging login ticket...");
  auto alive = alive_flag_;
  auth_api_.exchangeTicketAsync(
      ticket, [this, alive](bool ok, SocialAuthSession session, juce::String error_text) {
        if (!alive->load()) {
          return;
        }
        login_view_.setBusy(false);
        if (!ok) {
          login_view_.setStatusText("Ticket exchange failed: " + error_text);
          return;
        }
        onAuthSessionReady(session);
      });
}

void ClientAppPresenter::onAuthSessionReady(const SocialAuthSession& session) {
  auth_completed_ = true;
  oauth_login_pending_ = false;
  oauth_pending_provider_.clear();
  login_view_.setBusy(false);
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
  refreshProfile();
  fetchIceConfig();
  fetchRoomsFromApi();
  switchScreen(ScreenMode::Lobby);
}

void ClientAppPresenter::openRoomEntry(const std::string& room_code) {
  const RoomSummary* room = room_catalog_.findByCode(room_code);
  if (room == nullptr) {
    setLobbyStatusText(tempolink::juceapp::text::kRoomNotFound);
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
  room_entry_view_.setStatusText(tempolink::juceapp::text::kEntryPreflightGuide);
  switchScreen(ScreenMode::RoomEntry);
}

void ClientAppPresenter::previewRoom(const std::string& room_code) {
  openRoomEntry(room_code);
  room_entry_view_.setStatusText("PREVIEW selected. Monitor-only mode will be enabled in next step.");
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
  lobby_filter_ = filter;
  fetchRoomsFromApi();
}

void ClientAppPresenter::openRoom(const std::string& room_code) {
  if (!auth_completed_ || current_user_id_.empty()) {
    login_view_.setStatusText("Please sign in before joining rooms.");
    switchScreen(ScreenMode::Login);
    return;
  }

  leaveRoom();

  const RoomSummary* room = room_catalog_.findByCode(room_code);
  if (room == nullptr) {
    setLobbyStatusText(tempolink::juceapp::text::kRoomNotFound);
    return;
  }

  pending_room_code_ = room_code;
  const juce::String joining_text =
      juce::String(tempolink::juceapp::text::kJoiningRoomPrefix) +
      juce::String(room_code) +
      tempolink::juceapp::text::kJoinPendingSuffix;
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
                              const juce::String message =
                                  juce::String(
                                      tempolink::juceapp::text::kJoinFailedPrefix) +
                                  error_text;
                              setLobbyStatusText(message);
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
                              setLobbyStatusText(message);
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
                            config.room_id = tempolink::juceapp::app::HashToU32(room_code);
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
                            peer_latency_ms_.clear();
                            peer_ping_last_sent_ms_.clear();
                            last_peer_ping_tick_ = std::chrono::steady_clock::time_point::min();
                            rebuildParticipantsFromUserIds(room_after_join->participants);
                            refreshSessionAudioDevices();
                            refreshSessionViewState();
                            const bool signaling_ok = signaling_client_.connect(
                                control_plane_host_,
                                control_plane_port_,
                                room_code, current_user_id_,
                                [this, alive](const SignalingClient::Event& event) {
                                  if (!alive->load()) {
                                    return;
                                  }
                                  handleSignalingEvent(event);
                                });
                            if (!signaling_ok) {
                              session_view_.setStatusText(
                                  tempolink::juceapp::text::kMediaConnectedSignalingOffline);
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
  peer_latency_ms_.clear();
  peer_ping_last_sent_ms_.clear();
  last_peer_ping_tick_ = std::chrono::steady_clock::time_point::min();
  session_view_.setParticipants(participants_);
  session_view_.setConnectionState(false);
  session_view_.setStatusText(tempolink::juceapp::text::kSessionDisconnected);
}

void ClientAppPresenter::switchScreen(ScreenMode mode) {
  screen_mode_ = mode;
  const bool show_login = screen_mode_ == ScreenMode::Login;
  const bool show_lobby = screen_mode_ == ScreenMode::Lobby;
  const bool show_my_rooms = screen_mode_ == ScreenMode::MyRooms;
  const bool show_room_entry = screen_mode_ == ScreenMode::RoomEntry;
  const bool show_session = screen_mode_ == ScreenMode::Session;
  const bool show_profile = screen_mode_ == ScreenMode::Profile;
  const bool show_users = screen_mode_ == ScreenMode::Users;
  const bool show_news = screen_mode_ == ScreenMode::News;
  const bool show_manual = screen_mode_ == ScreenMode::Manual;
  const bool show_qna = screen_mode_ == ScreenMode::Qna;
  const bool show_settings = screen_mode_ == ScreenMode::Settings;
  login_view_.setVisible(show_login);
  lobby_view_.setVisible(show_lobby);
  my_rooms_view_.setVisible(show_my_rooms);
  room_entry_view_.setVisible(show_room_entry);
  session_view_.setVisible(show_session);
  profile_view_.setVisible(show_profile);
  users_view_.setVisible(show_users);
  news_view_.setVisible(show_news);
  manual_view_.setVisible(show_manual);
  qna_view_.setVisible(show_qna);
  settings_view_.setVisible(show_settings);
}

void ClientAppPresenter::openNavigationScreen(LobbyView::NavigationTarget target) {
  using Nav = LobbyView::NavigationTarget;

  switch (target) {
    case Nav::Rooms:
      switchScreen(ScreenMode::Lobby);
      return;
    case Nav::Profile:
      refreshProfile();
      switchScreen(ScreenMode::Profile);
      break;
    case Nav::Users:
      refreshUsers();
      switchScreen(ScreenMode::Users);
      break;
    case Nav::News:
      refreshNews();
      switchScreen(ScreenMode::News);
      break;
    case Nav::Manual:
      refreshManual();
      switchScreen(ScreenMode::Manual);
      break;
    case Nav::Qna:
      refreshQna();
      switchScreen(ScreenMode::Qna);
      break;
    case Nav::Settings:
      refreshProfile();
      refreshSettingsView();
      switchScreen(ScreenMode::Settings);
      break;
    default:
      switchScreen(ScreenMode::Lobby);
      return;
  }
}

void ClientAppPresenter::refreshUsers(const std::string& query) {
  users_view_.setStatusText("Loading users...");
  auto alive = alive_flag_;
  users_api_.fetchUsersAsync(query, [this, alive](bool ok, std::vector<UserSummary> users,
                                                  juce::String error_text) {
    if (!alive->load()) {
      return;
    }
    if (!ok) {
      users_view_.setStatusText("Users load failed: " + error_text);
      users_view_.setUsers({});
      return;
    }
    users_view_.setUsers(users);
    users_view_.setStatusText("Users: " + juce::String(static_cast<int>(users.size())));
  });
}

void ClientAppPresenter::refreshNews() {
  news_view_.setStatusText("Loading notices...");
  auto alive = alive_flag_;
  news_api_.fetchNewsAsync([this, alive](bool ok, std::vector<NewsItem> items,
                                         juce::String error_text) {
    if (!alive->load()) {
      return;
    }
    if (!ok) {
      news_view_.setStatusText("News load failed: " + error_text);
      news_view_.setItems({});
      return;
    }
    news_view_.setItems(items);
    news_view_.setStatusText("Notices: " + juce::String(static_cast<int>(items.size())));
  });
}

void ClientAppPresenter::refreshProfile() {
  if (current_user_id_.empty()) {
    profile_view_.setStatusText("Sign in first.");
    return;
  }
  profile_view_.setStatusText("Loading profile...");
  auto alive = alive_flag_;
  profile_api_.fetchProfileAsync(
      current_user_id_, [this, alive](bool ok, UserProfileModel profile, juce::String error_text) {
        if (!alive->load()) {
          return;
        }
        if (!ok) {
          profile_view_.setStatusText("Profile load failed: " + error_text);
          return;
        }
        if (profile.display_name.isNotEmpty()) {
          current_display_name_ = profile.display_name.toStdString();
        }
        current_bio_ = profile.bio.toStdString();
        profile_view_.setProfile(current_display_name_, current_bio_);
        settings_view_.setProfileFields(current_display_name_, current_bio_);
        profile_view_.setStatusText("Profile loaded");
      });
}

void ClientAppPresenter::refreshManual() {
  manual_view_.setStatusText("Loading manual document...");
  auto alive = alive_flag_;
  document_service_.loadApiOrDocumentAsync(
      control_plane_base_url_,
      tempolink::juceapp::constants::kManualPath,
      tempolink::juceapp::constants::kManualFallbackDocPath,
      [this, alive](bool ok, juce::String text, juce::String error_text) {
        if (!alive->load()) {
          return;
        }
        if (!ok) {
          manual_view_.setStatusText("Manual load failed: " + error_text);
          manual_view_.setDocumentText("");
          return;
        }
        manual_view_.setDocumentText(text);
        manual_view_.setStatusText("Manual loaded");
      });
}

void ClientAppPresenter::refreshQna() {
  qna_view_.setStatusText("Loading Q&A document...");
  auto alive = alive_flag_;
  document_service_.loadApiOrDocumentAsync(
      control_plane_base_url_,
      tempolink::juceapp::constants::kFaqPath,
      tempolink::juceapp::constants::kFaqFallbackDocPath,
      [this, alive](bool ok, juce::String text, juce::String error_text) {
        if (!alive->load()) {
          return;
        }
        if (!ok) {
          qna_view_.setStatusText("Q&A load failed: " + error_text);
          qna_view_.setDocumentText("");
          return;
        }
        qna_view_.setDocumentText(text);
        qna_view_.setStatusText("Q&A loaded");
      });
}

void ClientAppPresenter::saveProfileSettings(const std::string& display_name,
                                             const std::string& bio) {
  if (current_user_id_.empty()) {
    settings_view_.setStatusText("Sign in first.");
    return;
  }
  settings_view_.setStatusText("Saving profile...");
  auto alive = alive_flag_;
  profile_api_.updateProfileAsync(
      current_user_id_, display_name, bio,
      [this, alive](bool ok, UserProfileModel profile, juce::String error_text) {
        if (!alive->load()) {
          return;
        }
        if (!ok) {
          settings_view_.setStatusText("Profile save failed: " + error_text);
          return;
        }
        current_display_name_ = profile.display_name.toStdString();
        current_bio_ = profile.bio.toStdString();
        profile_view_.setProfile(current_display_name_, current_bio_);
        settings_view_.setProfileFields(current_display_name_, current_bio_);
        settings_view_.setStatusText("Profile saved");
      });
}

void ClientAppPresenter::applySettingsAudioFormat(int sample_rate_hz, int buffer_samples) {
  const bool ok = session_.ConfigureAudioFormat(
      static_cast<std::uint32_t>(sample_rate_hz),
      static_cast<std::uint16_t>(buffer_samples));
  if (ok) {
    settings_view_.setStatusText("Audio format applied: " + juce::String(sample_rate_hz) +
                                 " / " + juce::String(buffer_samples));
  } else {
    settings_view_.setStatusText("Failed to apply audio format");
  }
}

void ClientAppPresenter::refreshSettingsView() {
  settings_view_.setDevices(session_.AvailableInputDevices(), session_.SelectedInputDevice(),
                            session_.AvailableOutputDevices(),
                            session_.SelectedOutputDevice());
  settings_view_.setAudioFormat(static_cast<int>(session_.SampleRateHz()),
                                static_cast<int>(session_.FrameSamples()));
  settings_view_.setProfileFields(current_display_name_, current_bio_);
  settings_view_.setStatusText("Configure device/sample rate/buffer and apply.");
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

void ClientAppPresenter::refreshSessionViewState() {
  if (!session_active_) {
    return;
  }

  const auto& stats = session_.GetStats();
  const bool signaling_connected = signaling_client_.isConnected();
  tempolink::juceapp::app::RefreshSessionStatusView(
      session_view_, active_room_code_, stats, session_.AudioBackendName(),
      signaling_connected, participants_.size(), peer_latency_ms_,
      ice_config_loaded_, ice_config_);
  tempolink::juceapp::app::RefreshParticipantLevels(
      participants_, session_view_, tick_index_, stats, signaling_connected,
      peer_latency_ms_, ice_config_loaded_, ice_config_);
}

void ClientAppPresenter::sendPeerPings() {
  if (!session_active_ || !signaling_client_.isConnected()) {
    return;
  }
  const auto now = std::chrono::steady_clock::now();
  if (last_peer_ping_tick_ != std::chrono::steady_clock::time_point::min() &&
      now - last_peer_ping_tick_ < std::chrono::milliseconds(1000)) {
    return;
  }
  last_peer_ping_tick_ = now;
  const std::uint64_t sent_at_ms = NowSteadyMs();
  for (const auto& participant : participants_) {
    if (participant.is_self || participant.user_id.empty()) {
      continue;
    }
    if (signaling_client_.sendPeerPing(participant.user_id, sent_at_ms)) {
      peer_ping_last_sent_ms_[participant.user_id] = sent_at_ms;
    }
  }
}

void ClientAppPresenter::setLobbyStatusText(const juce::String& base_message) {
  if (!alive_flag_->load()) {
    return;
  }

  juce::String message = base_message;
  if (ice_config_loaded_) {
    message += " | net: ";
    message += (ice_config_.hasDirectAssist() ? "p2p-assist" : "limited-direct");
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

void ClientAppPresenter::rebuildParticipantsFromUserIds(
    const std::vector<std::string>& user_ids) {
  const bool self_muted = session_active_ ? session_.IsMuted() : false;
  tempolink::juceapp::app::RebuildParticipants(
      user_ids, current_user_id_, selected_part_label_, self_muted,
      participants_);
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
    rebuildParticipantsFromUserIds(event.participants);
    return;
  }

  if (event.type == SignalingClient::Event::Type::PeerJoined &&
      !event.user_id.empty()) {
    rebuildParticipantsFromUserIds(
        ParticipantRosterBuilder::WithJoined(participants_, event.user_id));
    return;
  }

  if (event.type == SignalingClient::Event::Type::PeerLeft &&
      !event.user_id.empty()) {
    peer_latency_ms_.erase(event.user_id);
    peer_ping_last_sent_ms_.erase(event.user_id);
    rebuildParticipantsFromUserIds(ParticipantRosterBuilder::WithoutUser(
        participants_, event.user_id, current_user_id_));
    return;
  }

  if (event.type == SignalingClient::Event::Type::PeerPing) {
    if (!event.from_user_id.empty() && event.sent_at_ms > 0) {
      signaling_client_.sendPeerPong(event.from_user_id, event.sent_at_ms);
    }
    return;
  }

  if (event.type == SignalingClient::Event::Type::PeerPong) {
    if (!event.from_user_id.empty() && event.sent_at_ms > 0) {
      const std::uint64_t now_ms = NowSteadyMs();
      if (now_ms >= event.sent_at_ms) {
        const auto rtt_ms = static_cast<int>(now_ms - event.sent_at_ms);
        peer_latency_ms_[event.from_user_id] = juce::jlimit(0, 5000, rtt_ms);
      }
    }
    return;
  }

  if (event.type == SignalingClient::Event::Type::Error) {
    session_view_.setStatusText(
        juce::String(tempolink::juceapp::text::kSignalingErrorPrefix) +
        event.message);
  }
}

void ClientAppPresenter::wireUiEvents() {
  login_view_.setOnLoginRequested(
      [this](std::string provider) { startOAuthLogin(provider); });
  login_view_.setOnRefreshProviders([this] { refreshAuthProviders(); });

  tempolink::juceapp::app::LobbyBindingsCallbacks lobby_callbacks;
  lobby_callbacks.on_preview_room = [this](std::string room_code) {
    previewRoom(room_code);
  };
  lobby_callbacks.on_enter_room = [this](std::string room_code) {
    openRoomEntry(room_code);
  };
  lobby_callbacks.on_create_room = [this](std::string host_user_id,
                                          int max_participants) {
    createRoomFromLobby(host_user_id, max_participants);
  };
  lobby_callbacks.on_filter_changed = [this](const LobbyRoomFilter& filter) {
    updateLobbyFilter(filter);
  };
  lobby_callbacks.on_open_my_rooms = [this] {
    my_rooms_view_.setStatusText("My rooms: " +
                                 juce::String(static_cast<int>(
                                     room_catalog_.ownedBy(current_user_id_).size())));
    switchScreen(ScreenMode::MyRooms);
  };
  lobby_callbacks.on_navigation_selected = [this](LobbyView::NavigationTarget target) {
    openNavigationScreen(target);
  };
  lobby_callbacks.on_my_rooms_back = [this] { switchScreen(ScreenMode::Lobby); };
  lobby_callbacks.on_my_rooms_preview = [this](std::string room_code) {
    previewRoom(room_code);
  };
  lobby_callbacks.on_my_rooms_enter = [this](std::string room_code) {
    openRoomEntry(room_code);
  };
  lobby_callbacks.on_my_rooms_edit = [this](std::string room_code) {
    editRoomFromMyRooms(room_code);
  };
  lobby_callbacks.on_my_rooms_delete = [this](std::string room_code) {
    deleteRoomFromMyRooms(room_code);
  };
  lobby_callbacks.on_my_rooms_share = [this](std::string room_code) {
    shareRoomFromMyRooms(room_code);
  };
  lobby_callbacks.on_my_rooms_create = [this](int max_participants) {
    createRoomFromLobby("", max_participants);
  };
  lobby_callbacks.on_my_rooms_refresh = [this] { fetchRoomsFromApi(); };
  lobby_callbacks.on_entry_back = [this] {
    entry_room_code_.clear();
    setLobbyStatusText("Entry cancelled.");
    switchScreen(ScreenMode::Lobby);
  };
  lobby_callbacks.on_entry_join =
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
      };
  lobby_callbacks.on_entry_part_changed = [this](std::string part_label) {
    selected_part_label_ = std::move(part_label);
  };
  lobby_callbacks.on_entry_input_changed = [this](std::string input_device) {
    preferred_input_device_ = std::move(input_device);
  };
  lobby_callbacks.on_entry_output_changed = [this](std::string output_device) {
    preferred_output_device_ = std::move(output_device);
  };
  lobby_callbacks.on_entry_open_audio_settings = [this] {
    room_entry_view_.setStatusText("Advanced audio settings will open in-session.");
  };
  tempolink::juceapp::app::BindLobbyViews(lobby_view_, my_rooms_view_,
                                          room_entry_view_, lobby_callbacks);

  tempolink::juceapp::app::SessionBindingsCallbacks session_callbacks;
  session_callbacks.on_back = [this] {
    leaveRoom();
    setLobbyStatusText("Returned to lobby.");
    switchScreen(ScreenMode::Lobby);
  };
  session_callbacks.on_disconnect = [this] {
    leaveRoom();
    setLobbyStatusText("Disconnected from room.");
    switchScreen(ScreenMode::Lobby);
  };
  session_callbacks.on_mute_changed = [this](bool muted) {
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
  };
  session_callbacks.on_metronome_changed = [this](bool enabled) {
    if (session_active_) {
      session_.SetMetronomeEnabled(enabled);
    }
  };
  session_callbacks.on_volume_changed = [this](float volume) {
    if (session_active_) {
      session_.SetVolume(volume);
    }
  };
  session_callbacks.on_bpm_changed = [this](int bpm) {
    if (session_active_) {
      session_.SetMetronomeBpm(bpm);
    }
  };
  session_callbacks.on_input_device_changed = [this](std::string device_id) {
    if (session_active_) {
      session_.SetInputDevice(device_id);
    }
  };
  session_callbacks.on_output_device_changed = [this](std::string device_id) {
    if (session_active_) {
      session_.SetOutputDevice(device_id);
    }
  };
  session_callbacks.on_open_audio_settings = [this] {
    session_view_.setStatusText("Audio settings dialog will be connected next.");
  };
  session_callbacks.on_participant_audio_settings = [this](std::string user_id) {
    if (user_id.empty()) {
      return;
    }
    session_view_.setStatusText("Participant audio settings: " +
                                juce::String(user_id));
  };
  session_callbacks.on_participant_reconnect = [this](std::string user_id) {
    if (!session_active_ || !signaling_client_.isConnected() || user_id.empty()) {
      return;
    }
    const std::uint64_t sent_at_ms = NowSteadyMs();
    if (signaling_client_.sendPeerPing(user_id, sent_at_ms)) {
      peer_ping_last_sent_ms_[user_id] = sent_at_ms;
      session_view_.setStatusText("Peer reconnect probe sent: " + juce::String(user_id));
    } else {
      session_view_.setStatusText("Peer reconnect probe failed: " + juce::String(user_id));
    }
  };
  tempolink::juceapp::app::BindSessionView(session_view_, session_callbacks);

  profile_view_.setOnBack([this] { switchScreen(ScreenMode::Lobby); });

  users_view_.setOnBack([this] { switchScreen(ScreenMode::Lobby); });
  users_view_.setOnRefresh([this](std::string query) { refreshUsers(query); });

  news_view_.setOnBack([this] { switchScreen(ScreenMode::Lobby); });
  news_view_.setOnRefresh([this] { refreshNews(); });

  manual_view_.setOnBack([this] { switchScreen(ScreenMode::Lobby); });
  manual_view_.setOnReload([this] { refreshManual(); });

  qna_view_.setOnBack([this] { switchScreen(ScreenMode::Lobby); });
  qna_view_.setOnReload([this] { refreshQna(); });

  settings_view_.setOnBack([this] { switchScreen(ScreenMode::Lobby); });
  settings_view_.setOnInputDeviceChanged([this](std::string input_device) {
    preferred_input_device_ = std::move(input_device);
    if (session_.SetInputDevice(preferred_input_device_)) {
      settings_view_.setStatusText("Input device updated.");
    } else {
      settings_view_.setStatusText("Failed to change input device.");
    }
  });
  settings_view_.setOnOutputDeviceChanged([this](std::string output_device) {
    preferred_output_device_ = std::move(output_device);
    if (session_.SetOutputDevice(preferred_output_device_)) {
      settings_view_.setStatusText("Output device updated.");
    } else {
      settings_view_.setStatusText("Failed to change output device.");
    }
  });
  settings_view_.setOnApplyAudio(
      [this](int sample_rate_hz, int buffer_samples) {
        applySettingsAudioFormat(sample_rate_hz, buffer_samples);
      });
  settings_view_.setOnSaveProfile(
      [this](std::string display_name, std::string bio) {
        saveProfileSettings(display_name, bio);
      });
}
