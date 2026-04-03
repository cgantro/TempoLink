#include "tempolink/juce/app/ControllerCompositionFactory.h"

#include <utility>

#include "tempolink/juce/app/auth/AuthController.h"
#include "tempolink/juce/app/content/ContentController.h"
#include "tempolink/juce/app/lobby/LobbyDataController.h"

namespace tempolink::juceapp::app {

RoomCommandUiCoordinator ControllerCompositionFactory::CreateRoomCommandUiCoordinator(
    tempolink::juceapp::di::ViewRegistry& views) {
  return RoomCommandUiCoordinator(views.my_rooms_view);
}

RoomCommandController ControllerCompositionFactory::CreateRoomCommandController(
    tempolink::juceapp::di::DependencyContainer& deps, RoomCatalog& room_catalog,
    AppStatusContext& status_context, RoomCommandController::Callbacks callbacks) {
  return RoomCommandController(
      RoomCommandController::Dependencies{
          deps.room_api, room_catalog, deps.alive_flag, status_context.current_user_id},
      std::move(callbacks));
}

AuthController ControllerCompositionFactory::CreateAuthController(
    tempolink::juceapp::di::DependencyContainer& deps,
    tempolink::juceapp::di::ViewRegistry& views, const std::string& callback_host,
    const std::uint16_t& callback_port, const std::string& callback_path) {
  return AuthController(deps.auth_api, deps.oauth_callback_server, views.login_view,
                        deps.alive_flag, callback_host, callback_port, callback_path);
}

LobbyDataController ControllerCompositionFactory::CreateLobbyDataController(
    tempolink::juceapp::di::DependencyContainer& deps, RoomCatalog& room_catalog,
    tempolink::juceapp::di::ViewRegistry& views, AppStatusContext& status_context,
    std::function<void(const std::string&)> set_lobby_status_text) {
  return LobbyDataController(deps.room_api, deps.ice_client, room_catalog, views.lobby_view,
                             views.my_rooms_view, deps.alive_flag, status_context,
                             std::move(set_lobby_status_text));
}

ContentController ControllerCompositionFactory::CreateContentController(
    tempolink::juceapp::di::DependencyContainer& deps,
    tempolink::juceapp::di::ViewRegistry& views, AppStatusContext& status_context) {
  return ContentController(
      views.users_view, views.news_view, views.profile_view, views.manual_view,
      views.qna_view, views.settings_view, deps.users_api, deps.news_api,
      deps.profile_api, deps.document_service, deps.session, deps.alive_flag,
      deps.control_plane_base_url, status_context.current_user_id,
      status_context.current_display_name, status_context.current_user_id);
}

SessionPresenceController
ControllerCompositionFactory::CreateSessionPresenceController(
    tempolink::juceapp::di::DependencyContainer& deps,
    tempolink::juceapp::di::ViewRegistry& views) {
  return SessionPresenceController(views.session_view, deps.session,
                                   deps.signaling_client);
}

SessionLifecycleController
ControllerCompositionFactory::CreateSessionLifecycleController(
    tempolink::juceapp::di::DependencyContainer& deps, RoomCatalog& room_catalog,
    AppStatusContext& status_context,
    SessionPresenceController& session_presence_controller,
    const SessionLifecycleCallbackBundle& callbacks) {
  return SessionLifecycleController(
      SessionLifecycleController::SessionServices{
          deps.session, deps.room_api, deps.signaling_client,
          session_presence_controller, room_catalog},
      status_context, callbacks.view_callbacks, callbacks.routing_callbacks,
      deps.alive_flag);
}

}  // namespace tempolink::juceapp::app
