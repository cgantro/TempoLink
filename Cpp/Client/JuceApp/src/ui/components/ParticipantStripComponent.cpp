#include "tempolink/juce/ui/components/ParticipantStripComponent.h"

#include <algorithm>
#include <utility>

#include <juce_events/juce_events.h>

#include "tempolink/juce/style/UiStyle.h"

namespace {

juce::String FormatLatencyMs(int latency_ms) {
  return latency_ms < 0 ? juce::String("-- ms")
                        : juce::String(latency_ms) + " ms";
}

}  // namespace

ParticipantStripComponent::ParticipantStripComponent() {
  name_label_.setFont(juce::FontOptions(15.0F).withStyle("Bold"));
  name_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());
  addAndMakeVisible(name_label_);

  part_label_.setJustificationType(juce::Justification::centredLeft);
  part_label_.setColour(juce::Label::textColourId,
                        tempolink::juceapp::style::TextSecondary());
  addAndMakeVisible(part_label_);

  state_label_.setJustificationType(juce::Justification::centredLeft);
  state_label_.setColour(juce::Label::textColourId,
                         tempolink::juceapp::style::TextSecondary());
  addAndMakeVisible(state_label_);

  level_slider_.setSliderStyle(juce::Slider::LinearHorizontal);
  level_slider_.setRange(0.0, 1.0, 0.001);
  level_slider_.setEnabled(false);
  level_slider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
  addAndMakeVisible(level_slider_);

  audio_settings_button_.onClick = [this] {
    const auto callback = on_audio_settings_clicked_;
    const auto user_id = participant_.user_id;
    juce::MessageManager::callAsync([callback, user_id]() {
      if (callback) {
        callback(user_id);
      }
    });
  };
  audio_settings_button_.setColour(juce::TextButton::buttonColourId,
                                   tempolink::juceapp::style::CardBackground());
  audio_settings_button_.setColour(juce::TextButton::textColourOffId,
                                   tempolink::juceapp::style::TextPrimary());
  addAndMakeVisible(audio_settings_button_);

  reconnect_button_.onClick = [this] {
    const auto callback = on_reconnect_clicked_;
    const auto user_id = participant_.user_id;
    juce::MessageManager::callAsync([callback, user_id]() {
      if (callback) {
        callback(user_id);
      }
    });
  };
  reconnect_button_.setColour(juce::TextButton::buttonColourId,
                              tempolink::juceapp::style::CardBackground());
  reconnect_button_.setColour(juce::TextButton::textColourOffId,
                              tempolink::juceapp::style::TextPrimary());
  addAndMakeVisible(reconnect_button_);

  latency_metric_.setTitle("Latency");
  latency_metric_.setValue("-- ms");
  addAndMakeVisible(latency_metric_);

  loss_metric_.setTitle("Loss");
  loss_metric_.setValue("-- %");
  addAndMakeVisible(loss_metric_);

  addAndMakeVisible(connection_badge_);
}

void ParticipantStripComponent::setParticipant(
    const ParticipantSummary& participant) {
  participant_ = participant;
  name_label_.setText(participant_.display_name, juce::dontSendNotification);

  juce::String part = participant_.part_label.isNotEmpty() ? participant_.part_label
                                                            : juce::String("Part -");
  part_label_.setText(part, juce::dontSendNotification);

  juce::String state = participant_.is_self ? "YOU" : "PEER";
  if (participant_.is_muted) {
    state += "  MUTED";
  }
  if (participant_.is_recording) {
    state += "  REC";
  }
  if (participant_.is_monitoring) {
    state += "  MON";
  }
  state_label_.setText(state, juce::dontSendNotification);

  latency_metric_.setValue(FormatLatencyMs(participant_.latency_ms));
  loss_metric_.setValue(
      juce::String(participant_.packet_loss_percent, 1) + " %");
  connection_badge_.setState(participant_.connection_state);
  reconnect_button_.setEnabled(!participant_.is_self);
  audio_settings_button_.setButtonText(participant_.is_self ? "Audio" : "Peer");
  setLevel(participant_.level);
}

void ParticipantStripComponent::setRuntimeMetrics(float level, int latency_ms,
                                                  float packet_loss_percent,
                                                  ConnectionBadgeState connection_state) {
  participant_.level = level;
  participant_.latency_ms = latency_ms;
  participant_.packet_loss_percent = packet_loss_percent;
  participant_.connection_state = connection_state;

  latency_metric_.setValue(FormatLatencyMs(participant_.latency_ms));
  loss_metric_.setValue(
      juce::String(participant_.packet_loss_percent, 1) + " %");
  connection_badge_.setState(participant_.connection_state);
  setLevel(participant_.level);
}

void ParticipantStripComponent::setLevel(float level) {
  level_slider_.setValue(std::clamp(level, 0.0F, 1.0F), juce::dontSendNotification);
}

void ParticipantStripComponent::setOnAudioSettingsClicked(
    std::function<void(std::string)> on_audio_settings_clicked) {
  on_audio_settings_clicked_ = std::move(on_audio_settings_clicked);
}

void ParticipantStripComponent::setOnReconnectClicked(
    std::function<void(std::string)> on_reconnect_clicked) {
  on_reconnect_clicked_ = std::move(on_reconnect_clicked);
}

void ParticipantStripComponent::resized() {
  auto area = getLocalBounds().reduced(10);
  const int actions_width = 170;
  auto action_area = area.removeFromRight(actions_width);
  audio_settings_button_.setBounds(action_area.removeFromTop(26).reduced(2, 0));
  action_area.removeFromTop(6);
  reconnect_button_.setBounds(action_area.removeFromTop(26).reduced(2, 0));

  auto top = area.removeFromTop(24);
  top.removeFromLeft(34);
  name_label_.setBounds(top.removeFromLeft(200));
  connection_badge_.setBounds(top.removeFromLeft(100));

  auto middle = area.removeFromTop(22);
  middle.removeFromLeft(34);
  part_label_.setBounds(middle.removeFromLeft(200));
  state_label_.setBounds(middle);

  area.removeFromTop(2);
  auto metrics = area.removeFromTop(28);
  metrics.removeFromLeft(34);
  latency_metric_.setBounds(metrics.removeFromLeft(120));
  metrics.removeFromLeft(8);
  loss_metric_.setBounds(metrics.removeFromLeft(90));

  area.removeFromTop(4);
  area.removeFromLeft(34);
  level_slider_.setBounds(area.removeFromTop(14));
}

void ParticipantStripComponent::paint(juce::Graphics& g) {
  auto bounds = getLocalBounds().toFloat().reduced(1.0F);
  g.setColour(tempolink::juceapp::style::ParticipantCardBackground());
  g.fillRoundedRectangle(bounds, 12.0F);
  g.setColour(tempolink::juceapp::style::PanelBorder());
  g.drawRoundedRectangle(bounds, 12.0F, 1.0F);

  const float avatar_size =
      static_cast<float>(tempolink::juceapp::style::kParticipantAvatarSize);
  const float avatar_x = 12.0F;
  const float avatar_y = 12.0F;
  g.setColour(participant_.is_self ? tempolink::juceapp::style::ParticipantAvatarSelf()
                                   : tempolink::juceapp::style::ParticipantAvatarPeer());
  g.fillEllipse(avatar_x, avatar_y, avatar_size, avatar_size);
  g.setColour(juce::Colours::white);
  const juce::String initial =
      participant_.display_name.isNotEmpty() ? participant_.display_name.substring(0, 1)
                                             : juce::String("?");
  g.setFont(juce::FontOptions(14.0F).withStyle("Bold"));
  g.drawFittedText(initial,
                   static_cast<int>(avatar_x),
                   static_cast<int>(avatar_y),
                   static_cast<int>(avatar_size),
                   static_cast<int>(avatar_size),
                   juce::Justification::centred, 1);
}
