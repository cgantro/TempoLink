#include "tempolink/juce/app/navigation/coordinators/AuthCoordinator.h"

#include <string>

#include "tempolink/juce/app/navigation/ScreenMode.h"

namespace tempolink::juceapp::app {

AuthCoordinator::AuthCoordinator(ScreenNavigator& navigator,
                                 tempolink::juceapp::app::AppStatusContext& context,
                                 LobbyDataController& lobby_controller)
    : navigator_(navigator),
      context_(context),
      lobby_controller_(lobby_controller) {}

void AuthCoordinator::start() {
  navigator_.SwitchTo(ScreenMode::Login);
}

void AuthCoordinator::onAuthSuccess(const SocialAuthSession& session) {
  context_.auth_completed = true;
  if (session.user_id.isNotEmpty()) {
    context_.current_user_id = session.user_id.toStdString();
  } else if (session.provider_user_id.isNotEmpty()) {
    context_.current_user_id = (session.provider + "-" + session.provider_user_id).toStdString();
  } else {
    // Basic fallback for anonymous login
    context_.current_user_id = "user-" + std::to_string(100000 + (std::rand() % 900000));
  }
  context_.current_display_name = session.display_name.isNotEmpty()
                               ? session.display_name.toStdString()
                               : context_.current_user_id;

  lobby_controller_.FetchIceConfig();
  lobby_controller_.FetchRooms();
  navigator_.SwitchTo(ScreenMode::Lobby);
}

}  // namespace tempolink::juceapp::app
