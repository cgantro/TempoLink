#include "tempolink/juce/ui/screens/ProfileView.h"

#include <utility>

#include "tempolink/juce/style/UiStyle.h"

ProfileView::ProfileView() {
  back_button_.onClick = [this] {
    if (on_back_) {
      on_back_();
    }
  };
  addAndMakeVisible(back_button_);

  title_label_.setText("My Profile", juce::dontSendNotification);
  title_label_.setFont(juce::FontOptions(26.0F).withStyle("Bold"));
  title_label_.setColour(juce::Label::textColourId,
                         tempolink::juceapp::style::TextPrimary());
  addAndMakeVisible(title_label_);

  status_label_.setColour(juce::Label::textColourId,
                          tempolink::juceapp::style::TextSecondary());
  status_label_.setText("Profile not loaded.", juce::dontSendNotification);
  addAndMakeVisible(status_label_);

  nickname_label_.setColour(juce::Label::textColourId,
                            tempolink::juceapp::style::TextPrimary());
  addAndMakeVisible(nickname_label_);

  nickname_value_.setColour(juce::Label::textColourId,
                            tempolink::juceapp::style::TextPrimary());
  addAndMakeVisible(nickname_value_);

  bio_label_.setColour(juce::Label::textColourId,
                       tempolink::juceapp::style::TextPrimary());
  addAndMakeVisible(bio_label_);

  bio_value_.setMultiLine(true);
  bio_value_.setReadOnly(true);
  bio_value_.setColour(juce::TextEditor::textColourId,
                       tempolink::juceapp::style::TextPrimary());
  bio_value_.setColour(juce::TextEditor::backgroundColourId,
                       tempolink::juceapp::style::PanelBackground());
  bio_value_.setColour(juce::TextEditor::outlineColourId,
                       tempolink::juceapp::style::BorderStrong());
  addAndMakeVisible(bio_value_);

  info_label_.setJustificationType(juce::Justification::topLeft);
  info_label_.setText(
      "Profile / social / favorites management area.\n"
      "- Nickname / bio\n"
      "- Favorites / block / report\n"
      "- Profile visibility settings",
      juce::dontSendNotification);
  addAndMakeVisible(info_label_);

  updateTheme();
}

void ProfileView::updateTheme() {
  title_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());
  status_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextSecondary());
  nickname_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());
  nickname_value_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());
  bio_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());
  info_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextSecondary());

  bio_value_.setColour(juce::TextEditor::textColourId, tempolink::juceapp::style::TextPrimary());
  bio_value_.setColour(juce::TextEditor::backgroundColourId, tempolink::juceapp::style::PanelBackground());
  bio_value_.setColour(juce::TextEditor::outlineColourId, tempolink::juceapp::style::BorderStrong());

  auto update_btn = [](juce::TextButton& btn) {
    btn.setColour(juce::TextButton::buttonColourId, tempolink::juceapp::style::PanelBackground());
    btn.setColour(juce::TextButton::textColourOffId, tempolink::juceapp::style::TextPrimary());
  };
  update_btn(back_button_);

  repaint();
}

void ProfileView::setOnBack(std::function<void()> on_back) {
  on_back_ = std::move(on_back);
}

void ProfileView::setProfile(const std::string& display_name, const std::string& bio) {
  nickname_value_.setText(juce::String(display_name), juce::dontSendNotification);
  bio_value_.setText(juce::String(bio), juce::dontSendNotification);
}

void ProfileView::setStatusText(const juce::String& status_text) {
  status_label_.setText(status_text, juce::dontSendNotification);
}

void ProfileView::resized() {
  auto area = getLocalBounds().reduced(18);
  auto top = area.removeFromTop(60);
  back_button_.setBounds(top.removeFromLeft(120).reduced(0, 14));
  top.removeFromLeft(10);
  title_label_.setBounds(top);
  area.removeFromTop(6);
  status_label_.setBounds(area.removeFromTop(24));
  area.removeFromTop(8);

  auto row = area.removeFromTop(28);
  nickname_label_.setBounds(row.removeFromLeft(110));
  nickname_value_.setBounds(row.removeFromLeft(360));
  area.removeFromTop(8);

  auto bio_row = area.removeFromTop(120);
  bio_label_.setBounds(bio_row.removeFromLeft(110));
  bio_value_.setBounds(bio_row);

  area.removeFromTop(8);
  info_label_.setBounds(area.reduced(4));
}

void ProfileView::paint(juce::Graphics& g) {
  g.fillAll(tempolink::juceapp::style::LobbyBackground());
}
