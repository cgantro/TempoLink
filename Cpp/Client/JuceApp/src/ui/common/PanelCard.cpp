#include "tempolink/juce/ui/common/PanelCard.h"

#include "tempolink/juce/style/UiStyle.h"

PanelCard::PanelCard() {
  title_label_.setFont(juce::FontOptions(14.0F).withStyle("Bold"));
  title_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());
  title_label_.setJustificationType(juce::Justification::centredLeft);
  addAndMakeVisible(title_label_);
}

void PanelCard::setTitle(const juce::String& title) {
  title_label_.setText(title, juce::dontSendNotification);
}

void PanelCard::setContent(juce::Component& content) {
  content_ = &content;
  if (content_->getParentComponent() != this) {
    addAndMakeVisible(content);
  } else {
    content_->setVisible(true);
  }
  resized();
}

void PanelCard::resized() {
  auto area = getLocalBounds().reduced(10);
  title_label_.setBounds(area.removeFromTop(20));
  area.removeFromTop(6);
  if (content_ != nullptr) {
    content_->setBounds(area);
  }
}

void PanelCard::paint(juce::Graphics& g) {
  const auto bounds = getLocalBounds().toFloat();
  g.setColour(tempolink::juceapp::style::PanelBackground());
  g.fillRoundedRectangle(bounds, 14.0F);
  g.setColour(tempolink::juceapp::style::PanelBorder());
  g.drawRoundedRectangle(bounds, 14.0F, 1.0F);
}
