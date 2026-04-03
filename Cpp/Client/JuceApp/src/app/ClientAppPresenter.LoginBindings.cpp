#include "tempolink/juce/app/ClientAppPresenter.h"

void ClientAppPresenter::wireLoginUiEvents() {
  views_.login_view.setOnLoginRequested(
      [this](std::string provider) { startOAuthLogin(provider); });
  views_.login_view.setOnRefreshProviders([this] { refreshAuthProviders(); });
}
