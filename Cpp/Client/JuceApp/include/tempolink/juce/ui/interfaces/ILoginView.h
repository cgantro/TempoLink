#pragma once

#include <string>
#include <vector>
#include <functional>

#include "tempolink/juce/network/auth/AuthApiClient.h"
#include "tempolink/juce/ui/models/UiModels.h"

class ILoginView {
 public:
  virtual ~ILoginView() = default;

  virtual void setOnLoginRequested(std::function<void(std::string)> on_login_requested) = 0;
  virtual void setOnRefreshProviders(std::function<void()> on_refresh_providers) = 0;

  virtual void setProviders(const std::vector<AuthProviderInfo>& providers) = 0;
  virtual void setStatusText(const std::string& status_text) = 0;
  virtual void setBusy(bool busy) = 0;
};
