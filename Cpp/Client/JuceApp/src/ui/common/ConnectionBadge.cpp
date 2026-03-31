#include "tempolink/juce/ui/common/ConnectionBadge.h"

#include "tempolink/juce/style/UiStyle.h"

ConnectionBadge::ConnectionBadge() = default;

void ConnectionBadge::setState(ConnectionBadgeState state) {
  if (state_ == state) {
    return;
  }
  state_ = state;
  repaint();
}

void ConnectionBadge::setText(const juce::String& text) {
  text_override_ = text;
  repaint();
}

void ConnectionBadge::paint(juce::Graphics& g) {
  const auto bounds = getLocalBounds().toFloat();
  g.setColour(tempolink::juceapp::style::ConnectionBadgeBackground(state_));
  g.fillRoundedRectangle(bounds, 10.0F);

  const juce::String label = text_override_.isNotEmpty()
                                 ? text_override_
                                 : tempolink::juceapp::style::ConnectionBadgeText(state_);
  g.setColour(tempolink::juceapp::style::TextInverted());
  g.setFont(juce::FontOptions(12.0F).withStyle("Bold"));
  g.drawFittedText(label, getLocalBounds().reduced(4), juce::Justification::centred, 1);
}
