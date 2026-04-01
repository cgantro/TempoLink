#pragma once

#include <functional>
#include <string>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "tempolink/juce/network/auth/AuthApiClient.h"

class LoginView final : public juce::Component {
 public:
  LoginView();

  void setOnLoginRequested(std::function<void(std::string)> on_login_requested);
  void setOnRefreshProviders(std::function<void()> on_refresh_providers);

  void setProviders(const std::vector<AuthProviderInfo>& providers);
  void setStatusText(const juce::String& status_text);
  void setBusy(bool busy);

  void resized() override;
  void paint(juce::Graphics& g) override;

 private:
  std::function<void(std::string)> on_login_requested_;
  std::function<void()> on_refresh_providers_;

  bool google_enabled_ = false;
  bool busy_ = false;

  juce::Label title_label_;
  juce::Label subtitle_label_;
  juce::Label status_label_;
  juce::TextButton google_login_button_{"Continue with Google"};
  juce::TextButton refresh_button_{"Refresh providers"};
};
