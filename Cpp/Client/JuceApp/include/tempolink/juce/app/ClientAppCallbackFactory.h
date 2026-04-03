#pragma once

#include <functional>
#include <string>

#include "tempolink/juce/app/RoomCommandController.h"
#include "tempolink/juce/app/navigation/ScreenMode.h"
#include "tempolink/juce/app/navigation/coordinators/RoomCommandUiCoordinator.h"
#include "tempolink/juce/app/session/SessionLifecycleController.h"
#include "tempolink/juce/di/ViewRegistry.h"

namespace tempolink::juceapp::app {

struct SessionLifecycleCallbackBundle {
  SessionLifecycleController::SessionViewCallbacks view_callbacks;
  SessionLifecycleController::SessionRoutingCallbacks routing_callbacks;
};

class ClientAppCallbackFactory {
 public:
  static RoomCommandController::Callbacks CreateRoomCommandCallbacks(
      tempolink::juceapp::di::ViewRegistry& views,
      RoomCommandUiCoordinator& room_command_ui_coordinator,
      std::function<void()> refresh_room_views,
      std::function<void()> fetch_rooms_from_api,
      std::function<void(const std::string&)> set_lobby_status_text,
      std::function<void(const std::string&)> open_room_entry);

  static SessionLifecycleCallbackBundle CreateSessionLifecycleCallbacks(
      tempolink::juceapp::di::ViewRegistry& views,
      std::function<void(const std::string&)> set_lobby_status_text,
      std::function<void(tempolink::juceapp::app::ScreenMode)> switch_screen,
      std::function<void()> on_session_started,
      std::function<void()> on_session_ended);
};

}  // namespace tempolink::juceapp::app
