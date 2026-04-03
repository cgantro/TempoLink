#pragma once

#include <cstdint>
#include <functional>
#include <string>

#include "tempolink/juce/app/AppStatusContext.h"
#include "tempolink/juce/app/ClientAppCallbackFactory.h"
#include "tempolink/juce/app/RoomCatalog.h"
#include "tempolink/juce/app/RoomCommandController.h"
#include "tempolink/juce/app/navigation/coordinators/RoomCommandUiCoordinator.h"
#include "tempolink/juce/app/session/SessionLifecycleController.h"
#include "tempolink/juce/app/session/SessionPresenceController.h"
#include "tempolink/juce/di/DependencyContainer.h"
#include "tempolink/juce/di/ViewRegistry.h"

class AuthController;
class ContentController;
class LobbyDataController;

namespace tempolink::juceapp::app {

class ControllerCompositionFactory {
 public:
  static RoomCommandUiCoordinator CreateRoomCommandUiCoordinator(
      tempolink::juceapp::di::ViewRegistry& views);

  static RoomCommandController CreateRoomCommandController(
      tempolink::juceapp::di::DependencyContainer& deps, RoomCatalog& room_catalog,
      AppStatusContext& status_context, RoomCommandController::Callbacks callbacks);

  static AuthController CreateAuthController(
      tempolink::juceapp::di::DependencyContainer& deps,
      tempolink::juceapp::di::ViewRegistry& views, const std::string& callback_host,
      const std::uint16_t& callback_port, const std::string& callback_path);

  static LobbyDataController CreateLobbyDataController(
      tempolink::juceapp::di::DependencyContainer& deps, RoomCatalog& room_catalog,
      tempolink::juceapp::di::ViewRegistry& views, AppStatusContext& status_context,
      std::function<void(const std::string&)> set_lobby_status_text);

  static ContentController CreateContentController(
      tempolink::juceapp::di::DependencyContainer& deps,
      tempolink::juceapp::di::ViewRegistry& views, AppStatusContext& status_context);

  static SessionPresenceController CreateSessionPresenceController(
      tempolink::juceapp::di::DependencyContainer& deps,
      tempolink::juceapp::di::ViewRegistry& views);

  static SessionLifecycleController CreateSessionLifecycleController(
      tempolink::juceapp::di::DependencyContainer& deps, RoomCatalog& room_catalog,
      AppStatusContext& status_context,
      SessionPresenceController& session_presence_controller,
      const SessionLifecycleCallbackBundle& callbacks);
};

}  // namespace tempolink::juceapp::app
