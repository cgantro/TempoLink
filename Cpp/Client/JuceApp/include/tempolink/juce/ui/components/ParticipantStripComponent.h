#pragma once

#include <functional>
#include <string>

#include <juce_gui_basics/juce_gui_basics.h>
#include "tempolink/juce/style/ThemeableComponent.h"

#include "tempolink/juce/ui/common/ConnectionBadge.h"
#include "tempolink/juce/ui/common/MetricLabel.h"
#include "tempolink/juce/ui/common/LevelMeterComponent.h"
#include "tempolink/juce/ui/models/UiModels.h"

class ParticipantStripComponent final : public tempolink::juceapp::style::ThemeableComponent {
 public:
  ParticipantStripComponent();
  void updateTheme() override;

  void setParticipant(const ParticipantSummary& participant);
  void setRuntimeMetrics(float level, int latency_ms, float packet_loss_percent,
                         ConnectionBadgeState connection_state);
  void setLevel(float level);
  void setMonitorMix(float monitor_volume, float monitor_pan);
  void setOnAudioSettingsClicked(
      std::function<void(std::string)> on_audio_settings_clicked);
  void setOnReconnectClicked(
      std::function<void(std::string)> on_reconnect_clicked);
  void setOnMonitorVolumeChanged(
      std::function<void(std::string, float)> on_monitor_volume_changed);
  void setOnMonitorPanChanged(
      std::function<void(std::string, float)> on_monitor_pan_changed);

  void resized() override;
  void paint(juce::Graphics& g) override;

 private:
  ParticipantSummary participant_;
  std::function<void(std::string)> on_audio_settings_clicked_;
  std::function<void(std::string)> on_reconnect_clicked_;
  std::function<void(std::string, float)> on_monitor_volume_changed_;
  std::function<void(std::string, float)> on_monitor_pan_changed_;

  juce::Label name_label_;
  juce::Label part_label_;
  juce::Label state_label_;
  tempolink::juceapp::ui::common::LevelMeterComponent level_meter_;
  juce::Slider monitor_volume_slider_;
  juce::Slider monitor_pan_slider_;
  juce::Label monitor_volume_label_;
  juce::Label monitor_pan_label_;
  juce::TextButton audio_settings_button_{"Audio"};
  juce::TextButton reconnect_button_{"Reconnect"};
  MetricLabel latency_metric_;
  MetricLabel loss_metric_;
  ConnectionBadge connection_badge_;
  bool suppress_monitor_callbacks_ = false;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParticipantStripComponent)
};
