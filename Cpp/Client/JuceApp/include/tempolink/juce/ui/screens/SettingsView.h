#pragma once

#include <functional>
#include <string>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>
#include "tempolink/juce/style/ThemeableComponent.h"

class SettingsView final : public tempolink::juceapp::style::ThemeableComponent {
 public:
  SettingsView();
  void updateTheme() override;

  void setOnBack(std::function<void()> on_back);
  void setOnInputDeviceChanged(std::function<void(std::string)> on_input_changed);
  void setOnOutputDeviceChanged(std::function<void(std::string)> on_output_changed);
  void setOnApplyAudio(std::function<void(int, int)> on_apply_audio);
  void setOnTestInput(std::function<void()> on_test_input);
  void setOnTestOutput(std::function<void()> on_test_output);
  void setOnSaveProfile(std::function<void(std::string, std::string)> on_save_profile);

  void setDevices(const std::vector<std::string>& input_devices,
                  const std::string& selected_input,
                  const std::vector<std::string>& output_devices,
                  const std::string& selected_output);
  void setAudioFormat(int sample_rate_hz, int buffer_samples);
  void setProfileFields(const std::string& display_name, const std::string& bio);
  void setStatusText(const juce::String& status_text);

  void resized() override;
  void paint(juce::Graphics& g) override;

 private:
  std::function<void()> on_back_;
  std::function<void(std::string)> on_input_changed_;
  std::function<void(std::string)> on_output_changed_;
  std::function<void(int, int)> on_apply_audio_;
  std::function<void()> on_test_input_;
  std::function<void()> on_test_output_;
  std::function<void(std::string, std::string)> on_save_profile_;

  juce::TextButton back_button_{"<- Rooms"};
  juce::Label title_label_;
  juce::Label status_label_;

  juce::Label input_label_{"", "Input"};
  juce::Label output_label_{"", "Output"};
  juce::Label sample_rate_label_{"", "Sample Rate"};
  juce::Label buffer_label_{"", "Buffer"};
  juce::Label nickname_label_{"", "Nickname"};
  juce::Label bio_label_{"", "Bio"};
  juce::Label theme_label_{"", "UI Theme"};

  juce::ComboBox input_combo_;
  juce::ComboBox output_combo_;
  juce::ComboBox sample_rate_combo_;
  juce::ComboBox buffer_combo_;
  juce::TextButton apply_button_{"Apply"};
  juce::TextButton test_input_button_{"Test Input"};
  juce::TextButton test_output_button_{"Test Output"};
  juce::TextEditor nickname_editor_;
  juce::TextEditor bio_editor_;
  juce::ComboBox theme_combo_;
  juce::TextButton save_profile_button_{"Save Profile"};
};
