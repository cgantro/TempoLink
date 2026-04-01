#pragma once

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "tempolink/juce/ui/common/PanelCard.h"

class MainOutPanel final : public juce::Component {
 public:
  MainOutPanel();

  void setMasterVolume(float volume);
  void setRecording(bool recording);
  void setAudioFileActive(bool active);
  void setOnVolumeChanged(std::function<void(float)> on_volume_changed);
  void setOnAudioFileToggle(std::function<void(bool)> on_audio_file_toggle);
  void setOnRecordToggle(std::function<void(bool)> on_record_toggle);
  void setOnDisconnect(std::function<void()> on_disconnect);

  void resized() override;

 private:
  bool suppress_callbacks_ = false;
  bool recording_ = false;
  bool audio_file_active_ = false;
  std::function<void(float)> on_volume_changed_;
  std::function<void(bool)> on_audio_file_toggle_;
  std::function<void(bool)> on_record_toggle_;
  std::function<void()> on_disconnect_;

  PanelCard card_;
  juce::Component body_;

  juce::Label master_label_;
  juce::Slider master_slider_;
  juce::TextButton audio_file_button_{"Audio File"};
  juce::TextButton record_button_{"Record"};
  juce::TextButton disconnect_button_{"EXIT"};
};
