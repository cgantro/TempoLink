#include "tempolink/juce/app/ClientAppPresenter.h"

void ClientAppPresenter::wireLoginUiEvents() {
  login_view_.setOnLoginRequested(
      [this](std::string provider) { startOAuthLogin(provider); });
  login_view_.setOnRefreshProviders([this] { refreshAuthProviders(); });
}
