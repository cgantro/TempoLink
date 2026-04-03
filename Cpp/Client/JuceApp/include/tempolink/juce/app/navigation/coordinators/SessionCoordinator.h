#pragma once

#include "tempolink/juce/app/navigation/ScreenNavigator.h"
#include "tempolink/juce/app/AppStatusContext.h"
#include "tempolink/juce/app/session/SessionLifecycleController.h"

namespace tempolink::juceapp::app {

class SessionCoordinator {
 public:
  SessionCoordinator(ScreenNavigator& navigator,
                     tempolink::juceapp::app::AppStatusContext& context,
                     SessionLifecycleController& session_controller);

  void startSession(const std::string& room_code, int tick_index,
                    bool ice_config_loaded,
                    const IceConfigSnapshot& ice_config);
  void leaveSession();
  void backToLobby();

 private:
  ScreenNavigator& navigator_;
  tempolink::juceapp::app::AppStatusContext& context_;
  SessionLifecycleController& session_controller_;
};

}  // namespace tempolink::juceapp::app
