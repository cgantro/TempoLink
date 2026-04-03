#pragma once

#include <functional>
#include <string>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>
#include "tempolink/juce/style/ThemeableComponent.h"

#include "tempolink/juce/ui/interfaces/ILoginView.h"

class LoginView final : public tempolink::juceapp::style::ThemeableComponent,
                        public ILoginView {
 public:
  LoginView();
  void updateTheme() override;

  void setOnLoginRequested(std::function<void(std::string)> on_login_requested) override;
  void setOnRefreshProviders(std::function<void()> on_refresh_providers) override;

  void setProviders(const std::vector<AuthProviderInfo>& providers) override;
  void setStatusText(const std::string& status_text) override;
  void setBusy(bool busy) override;

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

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoginView)
};
