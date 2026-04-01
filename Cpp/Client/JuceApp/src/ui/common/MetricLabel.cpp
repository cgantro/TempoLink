#include "tempolink/juce/ui/common/MetricLabel.h"

#include "tempolink/juce/style/UiStyle.h"

MetricLabel::MetricLabel() {
  tempolink::juceapp::style::ThemeManager::getInstance().addListener(this);
  title_label_.setJustificationType(juce::Justification::centredLeft);
  title_label_.setFont(juce::FontOptions(11.0F));
  addAndMakeVisible(title_label_);

  value_label_.setJustificationType(juce::Justification::centredLeft);
  value_label_.setFont(juce::FontOptions(13.0F).withStyle("Bold"));
  addAndMakeVisible(value_label_);
  
  updateTheme();
}

MetricLabel::~MetricLabel() {
  tempolink::juceapp::style::ThemeManager::getInstance().removeListener(this);
}

void MetricLabel::themeChanged() {
  updateTheme();
}

void MetricLabel::updateTheme() {
  title_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextSecondary());
  value_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());
  repaint();
}

void MetricLabel::setTitle(const juce::String& title) {
  title_label_.setText(title, juce::dontSendNotification);
}

void MetricLabel::setValue(const juce::String& value) {
  value_label_.setText(value, juce::dontSendNotification);
}

void MetricLabel::resized() {
  auto area = getLocalBounds();
  title_label_.setBounds(area.removeFromTop(14));
  value_label_.setBounds(area);
}
