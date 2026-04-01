#include "tempolink/juce/ui/session/TopStatusBar.h"

#include <utility>

#include "tempolink/juce/style/UiStyle.h"

TopStatusBar::TopStatusBar() {
  card_.setTitle("Session");
  addAndMakeVisible(card_);
  card_.setContent(body_);

  room_title_label_.setFont(juce::FontOptions(22.0F).withStyle("Bold"));
  room_title_label_.setText("Jam Session", juce::dontSendNotification);
  body_.addAndMakeVisible(room_title_label_);

  connection_badge_.setState(ConnectionBadgeState::Unknown);
  body_.addAndMakeVisible(connection_badge_);

  body_.addAndMakeVisible(status_label_);

  back_button_.onClick = [this] {
    if (on_back_) {
      on_back_();
    }
  };
  body_.addAndMakeVisible(back_button_);
  
  updateTheme();
}

void TopStatusBar::updateTheme() {
  room_title_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());
  status_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextSecondary());
  repaint();
}

void TopStatusBar::setRoomTitle(const juce::String& title) {
  room_title_label_.setText(title, juce::dontSendNotification);
}

void TopStatusBar::setStatusText(const juce::String& status) {
  status_label_.setText(status, juce::dontSendNotification);
}

void TopStatusBar::setConnectionMode(ConnectionBadgeState state) {
  connection_badge_.setState(state);
}

void TopStatusBar::setOnBack(std::function<void()> on_back) {
  on_back_ = std::move(on_back);
}

void TopStatusBar::resized() {
  card_.setBounds(getLocalBounds());
  auto area = body_.getLocalBounds();
  back_button_.setBounds(area.removeFromLeft(84));
  area.removeFromLeft(8);
  room_title_label_.setBounds(area.removeFromLeft(280));
  area.removeFromLeft(8);
  connection_badge_.setBounds(area.removeFromLeft(120));
  area.removeFromLeft(8);
  status_label_.setBounds(area);
}
