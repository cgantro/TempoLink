#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "tempolink/juce/app/auth/AuthController.h"
#include "tempolink/juce/app/ClientAppCallbackFactory.h"
#include "tempolink/juce/app/ControllerCompositionFactory.h"
#include "tempolink/juce/app/content/ContentController.h"
#include "tempolink/juce/app/lobby/LobbyDataController.h"
#include "tempolink/juce/app/navigation/ScreenMode.h"
#include "tempolink/juce/app/navigation/ScreenNavigator.h"
#include "tempolink/juce/app/RoomCatalog.h"
#include "tempolink/juce/app/RoomCommandController.h"
#include "tempolink/juce/app/session/SessionLifecycleController.h"
#include "tempolink/juce/app/session/SessionPresenceController.h"
#include "tempolink/juce/app/navigation/coordinators/AuthCoordinator.h"
#include "tempolink/juce/app/navigation/coordinators/LobbyCoordinator.h"
#include "tempolink/juce/app/navigation/coordinators/RoomCommandUiCoordinator.h"
#include "tempolink/juce/app/navigation/coordinators/SessionCoordinator.h"
#include "tempolink/juce/di/DependencyContainer.h"
#include "tempolink/juce/di/ViewRegistry.h"
#include "tempolink/juce/network/http/room/RoomApiTypes.h"
#include "tempolink/juce/app/AppStatusContext.h"
#include "tempolink/juce/constants/AuthConstants.h"
#include "tempolink/juce/network/auth/AuthApiClient.h"
#include "tempolink/juce/ui/models/UiModels.h"

namespace tempolink::juceapp::bridge {
class AudioSessionService;
}

using AppStatusContext = tempolink::juceapp::app::AppStatusContext;

namespace juce {
class FileChooser;
}

class ClientAppPresenter {
 public:
  ClientAppPresenter(tempolink::juceapp::di::ViewRegistry views,
                     tempolink::juceapp::di::DependencyContainer& deps);
  ~ClientAppPresenter();

  void initialize();
  void shutdown();
  void tick();
  void setAudioBridge(std::shared_ptr<tempolink::client::AudioBridgePort> audio_bridge);
  void setAudioSessionService(
      std::shared_ptr<tempolink::juceapp::bridge::AudioSessionService>
          audio_session_service);

 private:
  void openRoomEntry(const std::string& room_code);
  void refreshAuthProviders();
  void startOAuthLogin(const std::string& provider);
  void onAuthSessionReady(const SocialAuthSession& session);
  void previewRoom(const std::string& room_code);
  void createRoomFromLobby(const std::string& host_user_id,
                           const RoomCreatePayload& payload);
  void editRoomFromMyRooms(const std::string& room_code);
  void deleteRoomFromMyRooms(const std::string& room_code);
  void shareRoomFromMyRooms(const std::string& room_code);
  void updateLobbyFilter(const LobbyRoomFilter& filter);
  void fetchRoomsFromApi();
  void refreshRoomViews();
  void fetchIceConfig();
  void openRoom(const std::string& room_code);
  void leaveRoom();
  void refreshSessionAudioDevices();
  void openNavigationScreen(ILobbyView::NavigationTarget target);
  void setLobbyStatusText(const std::string& base_message);
  void wireUiEvents();
  void wireLoginUiEvents();
  void wireLobbyUiEvents();
  void wireSessionUiEvents();
  void wireUtilityUiEvents();

  // --- 1. Basic Injected dependencies ---
  tempolink::juceapp::di::ViewRegistry views_;
  tempolink::juceapp::di::DependencyContainer& deps_;

  // --- 2. Shared State ---
  AppStatusContext status_context_;
  std::string oauth_callback_host_ = tempolink::juceapp::constants::kOAuthCallbackHost;
  std::uint16_t oauth_callback_port_ = tempolink::juceapp::constants::kOAuthCallbackPort;
  std::string oauth_callback_path_ = tempolink::juceapp::constants::kOAuthCallbackPath;

  // --- 3. Core Controllers ---
  tempolink::juceapp::app::ScreenNavigator screen_navigator_;
  RoomCatalog room_catalog_;
  tempolink::juceapp::app::RoomCommandUiCoordinator room_command_ui_coordinator_;
  RoomCommandController room_commands_;
  AuthController auth_controller_;
  LobbyDataController lobby_data_controller_;
  ContentController content_controller_;
  SessionPresenceController session_presence_controller_;
  tempolink::juceapp::app::SessionLifecycleCallbackBundle
      session_lifecycle_callbacks_;
  SessionLifecycleController session_lifecycle_controller_;

  // --- 4. Navigation Coordinators (Depend on controllers) ---
  tempolink::juceapp::app::AuthCoordinator auth_coordinator_;
  tempolink::juceapp::app::LobbyCoordinator lobby_coordinator_;
  tempolink::juceapp::app::SessionCoordinator session_coordinator_;

  // --- Extra State ---
  bool session_recording_ = false;
  bool session_audio_file_active_ = false;
  std::shared_ptr<tempolink::juceapp::bridge::AudioSessionService>
      audio_session_service_;
  std::unique_ptr<juce::FileChooser> audio_file_chooser_;
  int tick_index_ = 0;

  juce::Component::SafePointer<LobbyView> lobby_view_safe_;
};
