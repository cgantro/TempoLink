#pragma once

#include <functional>
#include <string>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "tempolink/juce/ui/models/UiModels.h"
#include "tempolink/juce/ui/common/PanelCard.h"

class RoomEntrySettingsView final : public juce::Component {
 public:
  struct EntrySelection {
    std::string room_code;
    std::string part_label;
    std::string input_device;
    std::string output_device;
  };

  RoomEntrySettingsView();

  void setRoom(const RoomSummary& room);
  void setStatusText(const juce::String& status_text);
  void setInputDevices(const std::vector<std::string>& devices,
                       const std::string& selected_device);
  void setOutputDevices(const std::vector<std::string>& devices,
                        const std::string& selected_device);
  void setSelectedPart(const std::string& part_label);
  EntrySelection currentSelection() const;

  void setOnBack(std::function<void()> on_back);
  void setOnJoin(std::function<void(EntrySelection)> on_join);
  void setOnPartChanged(std::function<void(std::string)> on_part_changed);
  void setOnInputDeviceChanged(std::function<void(std::string)> on_input_changed);
  void setOnOutputDeviceChanged(std::function<void(std::string)> on_output_changed);
  void setOnOpenAudioSettings(std::function<void()> on_open_audio_settings);

  void resized() override;
  void paint(juce::Graphics& g) override;

 private:
  void updatePreflightState();
  static void PopulatePartOptions(juce::ComboBox& combo_box);

  bool suppress_callbacks_ = false;
  RoomSummary room_;

  std::function<void()> on_back_;
  std::function<void(EntrySelection)> on_join_;
  std::function<void(std::string)> on_part_changed_;
  std::function<void(std::string)> on_input_changed_;
  std::function<void(std::string)> on_output_changed_;
  std::function<void()> on_open_audio_settings_;

  PanelCard card_;
  juce::Component body_;

  juce::Label title_label_;
  juce::Label room_label_;
  juce::Label part_label_;
  juce::ComboBox part_combo_;
  juce::Label input_label_;
  juce::ComboBox input_combo_;
  juce::Label output_label_;
  juce::ComboBox output_combo_;
  juce::Label preflight_label_;
  juce::Label status_label_;
  juce::TextButton back_button_{"Back"};
  juce::TextButton audio_settings_button_{"Audio Settings"};
  juce::TextButton join_button_{"Join Session"};
};
