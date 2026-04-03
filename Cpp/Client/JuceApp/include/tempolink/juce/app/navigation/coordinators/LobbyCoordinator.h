#pragma once

#include <string>

#include "tempolink/juce/app/navigation/ScreenNavigator.h"
#include "tempolink/juce/app/content/ContentController.h"
#include "tempolink/juce/app/AppStatusContext.h"
#include "tempolink/juce/ui/interfaces/ILobbyView.h"

namespace tempolink::juceapp::app {

class LobbyCoordinator {
 public:
  LobbyCoordinator(ScreenNavigator& navigator, tempolink::juceapp::app::AppStatusContext& context,
                   ContentController& content_controller);

  void navigate(ILobbyView::NavigationTarget target);
  void switchToMyRooms();
  void switchToLobby();

 private:
  ScreenNavigator& navigator_;
  tempolink::juceapp::app::AppStatusContext& context_;
  ContentController& content_controller_;
};

}  // namespace tempolink::juceapp::app
