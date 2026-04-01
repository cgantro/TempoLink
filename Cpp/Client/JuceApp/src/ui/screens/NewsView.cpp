#include "tempolink/juce/ui/screens/NewsView.h"

#include <utility>

#include "tempolink/juce/style/UiStyle.h"

namespace {

juce::String FormatNews(const std::vector<NewsItem>& items) {
  juce::String text;
  for (const auto& item : items) {
    text << item.title << "\n";
    if (item.published_at.isNotEmpty()) {
      text << "  " << item.published_at << "\n";
    }
    if (item.body.isNotEmpty()) {
      text << item.body << "\n";
    }
    text << "\n";
  }
  return text.isEmpty() ? "No notices." : text;
}

}  // namespace

NewsView::NewsView() {
  back_button_.onClick = [this] {
    if (on_back_) {
      on_back_();
    }
  };
  addAndMakeVisible(back_button_);

  title_label_.setText("News", juce::dontSendNotification);
  title_label_.setFont(juce::FontOptions(26.0F).withStyle("Bold"));
  title_label_.setColour(juce::Label::textColourId,
                         tempolink::juceapp::style::TextPrimary());
  addAndMakeVisible(title_label_);

  status_label_.setColour(juce::Label::textColourId,
                          tempolink::juceapp::style::TextSecondary());
  addAndMakeVisible(status_label_);

  refresh_button_.onClick = [this] {
    if (on_refresh_) {
      on_refresh_();
    }
  };
  addAndMakeVisible(refresh_button_);

  news_text_.setMultiLine(true);
  news_text_.setReadOnly(true);
  news_text_.setScrollbarsShown(true);
  news_text_.setCaretVisible(false);
  addAndMakeVisible(news_text_);

  updateTheme();
}

void NewsView::updateTheme() {
  title_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());
  status_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextSecondary());
  
  auto update_btn = [](juce::TextButton& btn) {
    btn.setColour(juce::TextButton::buttonColourId, tempolink::juceapp::style::PanelBackground());
    btn.setColour(juce::TextButton::textColourOffId, tempolink::juceapp::style::TextPrimary());
  };
  update_btn(back_button_);
  update_btn(refresh_button_);

  news_text_.setColour(juce::TextEditor::textColourId, tempolink::juceapp::style::TextPrimary());
  news_text_.setColour(juce::TextEditor::backgroundColourId, tempolink::juceapp::style::PanelBackground());
  news_text_.setColour(juce::TextEditor::outlineColourId, tempolink::juceapp::style::BorderStrong());

  repaint();
}

void NewsView::setOnBack(std::function<void()> on_back) {
  on_back_ = std::move(on_back);
}

void NewsView::setOnRefresh(std::function<void()> on_refresh) {
  on_refresh_ = std::move(on_refresh);
}

void NewsView::setItems(const std::vector<NewsItem>& items) {
  news_text_.setText(FormatNews(items), juce::dontSendNotification);
}

void NewsView::setStatusText(const juce::String& status_text) {
  status_label_.setText(status_text, juce::dontSendNotification);
}

void NewsView::resized() {
  auto area = getLocalBounds().reduced(18);
  auto top = area.removeFromTop(60);
  back_button_.setBounds(top.removeFromLeft(120).reduced(0, 14));
  top.removeFromLeft(8);
  title_label_.setBounds(top);

  auto controls = area.removeFromTop(32);
  status_label_.setBounds(controls.removeFromLeft(280));
  controls.removeFromLeft(8);
  refresh_button_.setBounds(controls.removeFromLeft(110));

  area.removeFromTop(8);
  news_text_.setBounds(area);
}

void NewsView::paint(juce::Graphics& g) {
  g.fillAll(tempolink::juceapp::style::LobbyBackground());
}
