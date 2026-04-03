#pragma once

#include <string>
#include <functional>

#include "tempolink/juce/app/navigation/ScreenNavigator.h"
#include "tempolink/juce/app/AppStatusContext.h"
#include "tempolink/juce/app/lobby/LobbyDataController.h"
#include "tempolink/juce/app/auth/AuthController.h"

namespace tempolink::juceapp::app {

class AuthCoordinator {
 public:
  AuthCoordinator(ScreenNavigator& navigator,
                  tempolink::juceapp::app::AppStatusContext& context,
                  LobbyDataController& lobby_controller);

  void start();
  void onAuthSuccess(const SocialAuthSession& session);

 private:
  ScreenNavigator& navigator_;
  tempolink::juceapp::app::AppStatusContext& context_;
  LobbyDataController& lobby_controller_;
};

}  // namespace tempolink::juceapp::app
