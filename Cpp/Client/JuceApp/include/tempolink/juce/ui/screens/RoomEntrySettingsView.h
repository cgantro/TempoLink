#pragma once

#include <functional>
#include <string>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>
#include "tempolink/juce/style/ThemeableComponent.h"

#include "tempolink/juce/ui/common/PanelCard.h"
#include "tempolink/juce/ui/models/UiModels.h"

#include "tempolink/juce/ui/interfaces/IRoomEntryView.h"

class RoomEntrySettingsView final : public tempolink::juceapp::style::ThemeableComponent,
                                    public IRoomEntryView {
 public:
  RoomEntrySettingsView();
  void updateTheme() override;

  void setRoom(const RoomSummary& room) override;
  void setStatusText(const std::string& status_text) override;
  void setInputDevices(const std::vector<std::string>& devices,
                       const std::string& selected_device) override;
  void setOutputDevices(const std::vector<std::string>& devices,
                        const std::string& selected_device) override;
  void setSelectedPart(const std::string& part_label) override;
  EntrySelection currentSelection() const;

  void setOnBack(std::function<void()> on_back) override;
  void setOnJoin(std::function<void(EntrySelection)> on_join) override;
  void setOnPartChanged(std::function<void(std::string)> on_part_changed) override;
  void setOnInputDeviceChanged(std::function<void(std::string)> on_input_changed) override;
  void setOnOutputDeviceChanged(std::function<void(std::string)> on_output_changed) override;
  void setOnOpenAudioSettings(std::function<void()> on_open_audio_settings) override;

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
