#pragma once

#include <functional>
#include <string>

#include <juce_gui_basics/juce_gui_basics.h>
#include "tempolink/juce/style/ThemeableComponent.h"

class ProfileView final : public tempolink::juceapp::style::ThemeableComponent {
 public:
  ProfileView();
  void updateTheme() override;

  void setOnBack(std::function<void()> on_back);
  void setProfile(const std::string& display_name, const std::string& bio);
  void setStatusText(const juce::String& status_text);

  void resized() override;
  void paint(juce::Graphics& g) override;

 private:
  std::function<void()> on_back_;

  juce::TextButton back_button_{"<- Rooms"};
  juce::Label title_label_;
  juce::Label status_label_;
  juce::Label nickname_label_{"", "Nickname"};
  juce::Label bio_label_{"", "Bio"};
  juce::Label nickname_value_;
  juce::TextEditor bio_value_;
  juce::Label info_label_;
};
