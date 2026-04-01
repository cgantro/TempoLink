#include "tempolink/juce/ui/screens/UsersView.h"

#include <utility>

#include "tempolink/juce/style/UiStyle.h"

namespace {

juce::String FormatUsers(const std::vector<UserSummary>& users) {
  juce::String text;
  for (const auto& user : users) {
    text << "- " << user.display_name;
    if (!user.user_id.empty()) {
      text << " (" << user.user_id << ")";
    }
    text << "\n";
    if (user.bio.isNotEmpty()) {
      text << "  " << user.bio << "\n";
    }
    text << "\n";
  }
  return text.isEmpty() ? "No users found." : text;
}

}  // namespace

UsersView::UsersView() {
  back_button_.onClick = [this] {
    if (on_back_) {
      on_back_();
    }
  };
  addAndMakeVisible(back_button_);

  title_label_.setText("Users", juce::dontSendNotification);
  title_label_.setFont(juce::FontOptions(26.0F).withStyle("Bold"));
  title_label_.setColour(juce::Label::textColourId,
                         tempolink::juceapp::style::TextPrimary());
  addAndMakeVisible(title_label_);

  status_label_.setColour(juce::Label::textColourId,
                          tempolink::juceapp::style::TextSecondary());
  addAndMakeVisible(status_label_);

  search_editor_.setTextToShowWhenEmpty("search users", juce::Colours::grey);
  addAndMakeVisible(search_editor_);

  refresh_button_.onClick = [this] {
    if (on_refresh_) {
      on_refresh_(search_editor_.getText().trim().toStdString());
    }
  };
  addAndMakeVisible(refresh_button_);

  users_text_.setMultiLine(true);
  users_text_.setReadOnly(true);
  users_text_.setScrollbarsShown(true);
  users_text_.setCaretVisible(false);
  users_text_.setColour(juce::TextEditor::textColourId,
                        tempolink::juceapp::style::TextPrimary());
  users_text_.setColour(juce::TextEditor::backgroundColourId,
                        tempolink::juceapp::style::PanelBackground());
  users_text_.setColour(juce::TextEditor::outlineColourId,
                        tempolink::juceapp::style::BorderStrong());
  addAndMakeVisible(users_text_);
}

void UsersView::setOnBack(std::function<void()> on_back) {
  on_back_ = std::move(on_back);
}

void UsersView::setOnRefresh(std::function<void(std::string)> on_refresh) {
  on_refresh_ = std::move(on_refresh);
}

void UsersView::setUsers(const std::vector<UserSummary>& users) {
  users_text_.setText(FormatUsers(users), juce::dontSendNotification);
}

void UsersView::setStatusText(const juce::String& status_text) {
  status_label_.setText(status_text, juce::dontSendNotification);
}

void UsersView::resized() {
  auto area = getLocalBounds().reduced(18);
  auto top = area.removeFromTop(60);
  back_button_.setBounds(top.removeFromLeft(120).reduced(0, 14));
  top.removeFromLeft(8);
  title_label_.setBounds(top);

  auto controls = area.removeFromTop(32);
  status_label_.setBounds(controls.removeFromLeft(260));
  controls.removeFromLeft(8);
  search_editor_.setBounds(controls.removeFromLeft(260));
  controls.removeFromLeft(8);
  refresh_button_.setBounds(controls.removeFromLeft(110));

  area.removeFromTop(8);
  users_text_.setBounds(area);
}

void UsersView::paint(juce::Graphics& g) {
  g.fillAll(tempolink::juceapp::style::LobbyBackground());
}
