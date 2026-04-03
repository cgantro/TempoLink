#include "tempolink/juce/app/navigation/coordinators/SessionCoordinator.h"

#include <string>

#include "tempolink/juce/app/navigation/ScreenMode.h"

namespace tempolink::juceapp::app {

SessionCoordinator::SessionCoordinator(ScreenNavigator& navigator,
                                       tempolink::juceapp::app::AppStatusContext& context,
                                       SessionLifecycleController& session_controller)
    : navigator_(navigator),
      context_(context),
      session_controller_(session_controller) {}

void SessionCoordinator::startSession(const std::string& room_code, int tick_index,
                                      bool ice_config_loaded,
                                      const IceConfigSnapshot& ice_config) {
  session_controller_.OpenRoom(room_code, tick_index, ice_config_loaded, ice_config);
  // Navigation to Session screen is usually handled via callback in SessionLifecycleController,
  // but we can also trigger it here if we want more explicit control.
}

void SessionCoordinator::leaveSession() {
  session_controller_.LeaveRoom();
  navigator_.SwitchTo(ScreenMode::Lobby);
}

void SessionCoordinator::backToLobby() {
  navigator_.SwitchTo(ScreenMode::Lobby);
}

}  // namespace tempolink::juceapp::app
