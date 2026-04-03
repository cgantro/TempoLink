#include "tempolink/juce/ui/components/ParticipantStripComponent.h"

#include <algorithm>
#include <utility>
#include <juce_events/juce_events.h>
#include "tempolink/juce/style/UiStyle.h"

namespace {
juce::String FormatLatencyMs(int latency_ms) {
  return latency_ms < 0 ? juce::String("-- ms") : juce::String(latency_ms) + " ms";
}
}

ParticipantStripComponent::ParticipantStripComponent() {
  
  name_label_.setFont(juce::FontOptions(15.0F).withStyle("Bold"));
  addAndMakeVisible(name_label_);

  part_label_.setJustificationType(juce::Justification::centredLeft);
  addAndMakeVisible(part_label_);

  addAndMakeVisible(state_label_);
  addAndMakeVisible(level_meter_);

  monitor_volume_label_.setText("Vol", juce::dontSendNotification);
  monitor_volume_label_.setJustificationType(juce::Justification::centredLeft);
  addAndMakeVisible(monitor_volume_label_);

  monitor_volume_slider_.setSliderStyle(juce::Slider::LinearHorizontal);
  monitor_volume_slider_.setRange(0.0, 1.5, 0.01);
  monitor_volume_slider_.setValue(1.0, juce::dontSendNotification);
  monitor_volume_slider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 48, 18);
  monitor_volume_slider_.onValueChange = [this] {
    if (suppress_monitor_callbacks_) return;
    participant_.monitor_volume = static_cast<float>(monitor_volume_slider_.getValue());
    if (on_monitor_volume_changed_ && !participant_.user_id.empty()) {
      on_monitor_volume_changed_(participant_.user_id, participant_.monitor_volume);
    }
  };
  addAndMakeVisible(monitor_volume_slider_);

  monitor_pan_label_.setText("Pan", juce::dontSendNotification);
  monitor_pan_label_.setJustificationType(juce::Justification::centredLeft);
  addAndMakeVisible(monitor_pan_label_);

  monitor_pan_slider_.setSliderStyle(juce::Slider::LinearHorizontal);
  monitor_pan_slider_.setRange(-1.0, 1.0, 0.01);
  monitor_pan_slider_.setValue(0.0, juce::dontSendNotification);
  monitor_pan_slider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 48, 18);
  monitor_pan_slider_.onValueChange = [this] {
    if (suppress_monitor_callbacks_) return;
    participant_.monitor_pan = static_cast<float>(monitor_pan_slider_.getValue());
    if (on_monitor_pan_changed_ && !participant_.user_id.empty()) {
      on_monitor_pan_changed_(participant_.user_id, participant_.monitor_pan);
    }
  };
  addAndMakeVisible(monitor_pan_slider_);

  addAndMakeVisible(audio_settings_button_);
  audio_settings_button_.onClick = [this] { if (on_audio_settings_clicked_) on_audio_settings_clicked_(participant_.user_id); };
  
  addAndMakeVisible(reconnect_button_);
  reconnect_button_.onClick = [this] { if (on_reconnect_clicked_) on_reconnect_clicked_(participant_.user_id); };

  addAndMakeVisible(latency_metric_);
  
  loss_metric_.setTitle("Loss");
  addAndMakeVisible(loss_metric_);
  
  addAndMakeVisible(connection_badge_);
  
  updateTheme();
}
void ParticipantStripComponent::updateTheme() {
  name_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());
  part_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextSecondary());
  state_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextSecondary());
  monitor_volume_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextSecondary());
  monitor_pan_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextSecondary());

  auto update_btn = [](juce::TextButton& btn) {
    btn.setColour(juce::TextButton::buttonColourId, tempolink::juceapp::style::CardBackground());
    btn.setColour(juce::TextButton::textColourOffId, tempolink::juceapp::style::TextPrimary());
  };
  update_btn(audio_settings_button_);
  update_btn(reconnect_button_);

  auto update_slider = [](juce::Slider& s) {
    s.setColour(juce::Slider::thumbColourId, tempolink::juceapp::style::AccentCyan());
    s.setColour(juce::Slider::trackColourId, tempolink::juceapp::style::PanelBorder());
    s.setColour(juce::Slider::textBoxTextColourId, tempolink::juceapp::style::TextPrimary());
    s.setColour(juce::Slider::textBoxBackgroundColourId, tempolink::juceapp::style::CardBackground());
    s.setColour(juce::Slider::textBoxOutlineColourId, tempolink::juceapp::style::PanelBorder());
  };
  update_slider(monitor_volume_slider_);
  update_slider(monitor_pan_slider_);

  repaint();
}

void ParticipantStripComponent::setParticipant(const ParticipantSummary& participant) {
  participant_ = participant;
  name_label_.setText(participant_.display_name, juce::dontSendNotification);
  juce::String part = !participant_.part_label.empty() ? participant_.part_label : "Part -";
  part_label_.setText(part, juce::dontSendNotification);
  juce::String state = participant_.is_self ? "YOU" : "PEER";
  if (participant_.is_muted) state += " (MUTED)";
  state_label_.setText(state, juce::dontSendNotification);
  repaint();
}

void ParticipantStripComponent::setRuntimeMetrics(float level, int latency_ms, float packet_loss_percent, ConnectionBadgeState connection_state) {
  level_meter_.setLevel(level);
  latency_metric_.setValue(FormatLatencyMs(latency_ms));
  loss_metric_.setValue(juce::String(packet_loss_percent, 1) + "%");
  connection_badge_.setState(connection_state);
}

void ParticipantStripComponent::setLevel(float level) { level_meter_.setLevel(level); }

void ParticipantStripComponent::setMonitorMix(float monitor_volume, float monitor_pan) {
  suppress_monitor_callbacks_ = true;
  monitor_volume_slider_.setValue(monitor_volume, juce::dontSendNotification);
  monitor_pan_slider_.setValue(monitor_pan, juce::dontSendNotification);
  suppress_monitor_callbacks_ = false;
}

void ParticipantStripComponent::setOnAudioSettingsClicked(std::function<void(std::string)> on_audio_settings_clicked) { on_audio_settings_clicked_ = std::move(on_audio_settings_clicked); }
void ParticipantStripComponent::setOnReconnectClicked(std::function<void(std::string)> on_reconnect_clicked) { on_reconnect_clicked_ = std::move(on_reconnect_clicked); }
void ParticipantStripComponent::setOnMonitorVolumeChanged(std::function<void(std::string, float)> on_monitor_volume_changed) { on_monitor_volume_changed_ = std::move(on_monitor_volume_changed); }
void ParticipantStripComponent::setOnMonitorPanChanged(std::function<void(std::string, float)> on_monitor_pan_changed) { on_monitor_pan_changed_ = std::move(on_monitor_pan_changed); }

void ParticipantStripComponent::resized() {
  auto area = getLocalBounds().reduced(12, 8);
  
  // Left: Identity (Vertical Stack)
  auto left = area.removeFromLeft(150);
  name_label_.setBounds(left.removeFromTop(24));
  part_label_.setBounds(left.removeFromTop(20));
  state_label_.setBounds(left.removeFromTop(20));
  
  area.removeFromLeft(20);
  
  // Right: Actions (Buttons & Badge) - Thinner and more focused
  auto right = area.removeFromRight(180);
  auto action_row = right.removeFromTop(32);
  audio_settings_button_.setBounds(action_row.removeFromLeft(50).reduced(0, 4));
  action_row.removeFromLeft(4);
  reconnect_button_.setBounds(action_row.removeFromLeft(70).reduced(0, 4));
  action_row.removeFromLeft(4);
  connection_badge_.setBounds(action_row.reduced(0, 4));
  
  area.removeFromRight(20);
  
  // Center: Metrics (Top) & Vertical Stack (Vol -> Pan -> Level Meter)
  auto center = area;
  auto metrics = center.removeFromTop(24);
  latency_metric_.setBounds(metrics.removeFromLeft(80));
  metrics.removeFromLeft(10);
  loss_metric_.setBounds(metrics.removeFromLeft(80));
  
  center.removeFromTop(6);
  
  // Three rows stacking vertically with same width
  const int row_height = 24;
  const int row_spacing = 6;
  
  auto vol_row = center.removeFromTop(row_height);
  monitor_volume_label_.setBounds(vol_row.removeFromLeft(30));
  monitor_volume_slider_.setBounds(vol_row);
  
  center.removeFromTop(row_spacing);
  
  auto pan_row = center.removeFromTop(row_height);
  monitor_pan_label_.setBounds(pan_row.removeFromLeft(30));
  monitor_pan_slider_.setBounds(pan_row);
  
  center.removeFromTop(row_spacing);
  
  // Level meter at the bottom, matching the width of the sliders above
  level_meter_.setBounds(center.removeFromTop(18));
}

void ParticipantStripComponent::paint(juce::Graphics& g) {
  auto bounds = getLocalBounds().toFloat().reduced(0.5f);
  g.setColour(tempolink::juceapp::style::ParticipantCardBackground());
  g.fillRoundedRectangle(bounds, 10.0f);
  g.setColour(tempolink::juceapp::style::PanelBorder());
  g.drawRoundedRectangle(bounds, 10.0f, 1.0f);
}
