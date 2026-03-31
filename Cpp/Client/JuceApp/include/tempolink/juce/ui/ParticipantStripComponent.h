#pragma once

#include <functional>
#include <string>

#include <juce_gui_basics/juce_gui_basics.h>

#include "tempolink/juce/ui/common/ConnectionBadge.h"
#include "tempolink/juce/ui/common/MetricLabel.h"
#include "tempolink/juce/ui/UiModels.h"

class ParticipantStripComponent final : public juce::Component {
 public:
  ParticipantStripComponent();

  void setParticipant(const ParticipantSummary& participant);
  void setLevel(float level);
  void setOnAudioSettingsClicked(
      std::function<void(std::string)> on_audio_settings_clicked);
  void setOnReconnectClicked(
      std::function<void(std::string)> on_reconnect_clicked);

  void resized() override;
  void paint(juce::Graphics& g) override;

 private:
  ParticipantSummary participant_;
  std::function<void(std::string)> on_audio_settings_clicked_;
  std::function<void(std::string)> on_reconnect_clicked_;

  juce::Label name_label_;
  juce::Label part_label_;
  juce::Label state_label_;
  juce::Slider level_slider_;
  juce::TextButton audio_settings_button_{"Audio"};
  juce::TextButton reconnect_button_{"Reconnect"};
  MetricLabel latency_metric_;
  MetricLabel loss_metric_;
  ConnectionBadge connection_badge_;
};
