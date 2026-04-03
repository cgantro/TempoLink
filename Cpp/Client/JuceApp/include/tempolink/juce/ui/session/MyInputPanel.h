#pragma once

#include <functional>
#include <string>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>
#include "tempolink/juce/style/ThemeableComponent.h"

#include "tempolink/juce/ui/common/PanelCard.h"
#include "tempolink/juce/ui/common/LevelMeterComponent.h"

class MyInputPanel final : public tempolink::juceapp::style::ThemeableComponent {
 public:
  MyInputPanel();
  void updateTheme() override;

  void setInputDevices(const std::vector<std::string>& devices, const std::string& selected);
  void setOutputDevices(const std::vector<std::string>& devices, const std::string& selected);
  void setMute(bool muted);
  void setInputLevel(float level);
  void setInputGain(float gain);
  void setInputReverb(float amount);
  void setMetronomeEnabled(bool enabled);
  void setMetronomeBpm(int bpm);
  void setMetronomeTone(int tone);

  void setOnMuteChanged(std::function<void(bool)> on_mute_changed);
  void setOnInputGainChanged(std::function<void(float)> on_input_gain_changed);
  void setOnInputReverbChanged(std::function<void(float)> on_input_reverb_changed);
  void setOnMetronomeChanged(std::function<void(bool)> on_metronome_changed);
  void setOnBpmChanged(std::function<void(int)> on_bpm_changed);
  void setOnMetronomeToneChanged(std::function<void(int)> on_metronome_tone_changed);
  void setOnInputDeviceChanged(std::function<void(std::string)> on_input_changed);
  void setOnOutputDeviceChanged(std::function<void(std::string)> on_output_changed);
  void setOnOpenAudioSettings(std::function<void()> on_open_audio_settings);

  void resized() override;

 private:
  bool suppress_callbacks_ = false;

  std::function<void(bool)> on_mute_changed_;
  std::function<void(float)> on_input_gain_changed_;
  std::function<void(float)> on_input_reverb_changed_;
  std::function<void(bool)> on_metronome_changed_;
  std::function<void(int)> on_bpm_changed_;
  std::function<void(int)> on_metronome_tone_changed_;
  std::function<void(std::string)> on_input_changed_;
  std::function<void(std::string)> on_output_changed_;
  std::function<void()> on_open_audio_settings_;

  PanelCard card_;
  juce::Component body_;

  juce::Label input_label_;
  juce::ComboBox input_combo_;
  juce::Label output_label_;
  juce::ComboBox output_combo_;
  juce::TextButton mute_toggle_{"Mute Mic"};
  juce::Label input_level_label_;
  tempolink::juceapp::ui::common::LevelMeterComponent level_meter_;
  juce::Label input_gain_label_;
  juce::Slider input_gain_slider_;
  juce::Label reverb_label_;
  juce::Slider reverb_slider_;
  juce::TextButton metronome_toggle_{"Metronome"};
  juce::Label bpm_label_;
  juce::Slider bpm_slider_;
  juce::Label metronome_tone_label_;
  juce::ComboBox metronome_tone_combo_;
  juce::TextButton bpm_minus_button_{"-"};
  juce::TextButton bpm_plus_button_{"+"};
  juce::TextButton audio_settings_button_{"Audio Settings"};
};
