#include "tempolink/juce/app/navigation/coordinators/LobbyCoordinator.h"

#include "tempolink/juce/app/navigation/ScreenMode.h"

namespace tempolink::juceapp::app {

LobbyCoordinator::LobbyCoordinator(ScreenNavigator& navigator,
                                   tempolink::juceapp::app::AppStatusContext& context,
                                   ContentController& content_controller)
    : navigator_(navigator),
      context_(context),
      content_controller_(content_controller) {}

void LobbyCoordinator::navigate(ILobbyView::NavigationTarget target) {
  using Nav = ILobbyView::NavigationTarget;

  switch (target) {
    case Nav::Rooms:
      navigator_.SwitchTo(ScreenMode::Lobby);
      return;
    case Nav::Profile:
      content_controller_.refreshProfile();
      navigator_.SwitchTo(ScreenMode::Profile);
      break;
    case Nav::Users:
      content_controller_.refreshUsers();
      navigator_.SwitchTo(ScreenMode::Users);
      break;
    case Nav::News:
      content_controller_.refreshNews();
      navigator_.SwitchTo(ScreenMode::News);
      break;
    case Nav::Manual:
      content_controller_.refreshManual();
      navigator_.SwitchTo(ScreenMode::Manual);
      break;
    case Nav::Qna:
      content_controller_.refreshQna();
      navigator_.SwitchTo(ScreenMode::Qna);
      break;
    case Nav::Settings:
      content_controller_.refreshProfile();
      content_controller_.refreshSettingsView();
      navigator_.SwitchTo(ScreenMode::Settings);
      break;
    default:
      navigator_.SwitchTo(ScreenMode::Lobby);
      break;
  }
}

void LobbyCoordinator::switchToMyRooms() {
  navigator_.SwitchTo(ScreenMode::MyRooms);
}

void LobbyCoordinator::switchToLobby() {
  navigator_.SwitchTo(ScreenMode::Lobby);
}

}  // namespace tempolink::juceapp::app
