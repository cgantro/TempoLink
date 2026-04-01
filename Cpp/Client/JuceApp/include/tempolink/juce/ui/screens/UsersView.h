#pragma once

#include <functional>
#include <string>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>
#include "tempolink/juce/style/ThemeableComponent.h"

#include "tempolink/juce/network/http/user/UsersApiClient.h"

class UsersView final : public tempolink::juceapp::style::ThemeableComponent {
 public:
  UsersView();
  void updateTheme() override;

  void setOnBack(std::function<void()> on_back);
  void setOnRefresh(std::function<void(std::string)> on_refresh);
  void setUsers(const std::vector<UserSummary>& users);
  void setStatusText(const juce::String& status_text);

  void resized() override;
  void paint(juce::Graphics& g) override;

 private:
  std::function<void()> on_back_;
  std::function<void(std::string)> on_refresh_;

  juce::TextButton back_button_{"<- Rooms"};
  juce::Label title_label_;
  juce::Label status_label_;
  juce::TextEditor search_editor_;
  juce::TextButton refresh_button_{"Refresh"};
  juce::TextEditor users_text_;
};
