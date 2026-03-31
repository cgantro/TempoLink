#pragma once

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "tempolink/juce/ui/common/PanelCard.h"

class MainOutPanel final : public juce::Component {
 public:
  MainOutPanel();

  void setMasterVolume(float volume);
  void setOnVolumeChanged(std::function<void(float)> on_volume_changed);
  void setOnDisconnect(std::function<void()> on_disconnect);

  void resized() override;

 private:
  bool suppress_callbacks_ = false;
  std::function<void(float)> on_volume_changed_;
  std::function<void()> on_disconnect_;

  PanelCard card_;
  juce::Component body_;

  juce::Label master_label_;
  juce::Slider master_slider_;
  juce::TextButton disconnect_button_{"Disconnect"};
};
