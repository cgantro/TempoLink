#include "tempolink/juce/ui/RoomCardComponent.h"

#include <utility>

#include "tempolink/juce/style/UiStyle.h"

RoomCardComponent::RoomCardComponent() {
  title_label_.setFont(juce::FontOptions(16.0F).withStyle("Bold"));
  title_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());
  meta_label_.setJustificationType(juce::Justification::centredLeft);
  meta_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextSecondary());
  host_label_.setJustificationType(juce::Justification::centredLeft);
  host_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextSecondary());
  latency_label_.setJustificationType(juce::Justification::centredLeft);
  latency_label_.setColour(juce::Label::textColourId,
                           tempolink::juceapp::style::TextSecondary());

  join_button_.onClick = [this] {
    if (on_join_) {
      on_join_(room_.room_code);
    }
  };

  addAndMakeVisible(title_label_);
  addAndMakeVisible(meta_label_);
  addAndMakeVisible(host_label_);
  addAndMakeVisible(latency_label_);
  addAndMakeVisible(join_button_);
}

void RoomCardComponent::setRoom(const RoomSummary& room) {
  room_ = room;
  title_label_.setText(room_.title, juce::dontSendNotification);
  meta_label_.setText(
      room_.genre + "  |  " + juce::String(room_.members) + "/" +
          juce::String(room_.capacity),
      juce::dontSendNotification);
  host_label_.setText("Host: " + room_.host_label, juce::dontSendNotification);
  latency_label_.setText("Target latency: " + room_.latency_hint,
                         juce::dontSendNotification);
  repaint();
}

void RoomCardComponent::setJoinHandler(
    std::function<void(std::string)> on_join) {
  on_join_ = std::move(on_join);
}

void RoomCardComponent::resized() {
  auto area = getLocalBounds().reduced(12);
  title_label_.setBounds(area.removeFromTop(26));
  meta_label_.setBounds(area.removeFromTop(24));
  host_label_.setBounds(area.removeFromTop(22));
  latency_label_.setBounds(area.removeFromTop(22));
  join_button_.setBounds(area.removeFromBottom(30).removeFromRight(84));
}

void RoomCardComponent::paint(juce::Graphics& g) {
  auto bounds = getLocalBounds().toFloat().reduced(1.0F);
  g.setColour(tempolink::juceapp::style::CardBackground());
  g.fillRoundedRectangle(bounds, 14.0F);
  g.setColour(tempolink::juceapp::style::CardBorder());
  g.drawRoundedRectangle(bounds, 14.0F, 1.0F);
}
