#include "tempolink/juce/ui/screens/QnaView.h"

#include <utility>

#include "tempolink/juce/style/UiStyle.h"

QnaView::QnaView() {
  back_button_.onClick = [this] {
    if (on_back_) {
      on_back_();
    }
  };
  addAndMakeVisible(back_button_);

  reload_button_.onClick = [this] {
    if (on_reload_) {
      on_reload_();
    }
  };
  addAndMakeVisible(reload_button_);

  title_label_.setText("Q&A", juce::dontSendNotification);
  title_label_.setFont(juce::FontOptions(26.0F).withStyle("Bold"));
  title_label_.setColour(juce::Label::textColourId,
                         tempolink::juceapp::style::TextPrimary());
  addAndMakeVisible(title_label_);

  status_label_.setColour(juce::Label::textColourId,
                          tempolink::juceapp::style::TextSecondary());
  addAndMakeVisible(status_label_);

  doc_text_.setMultiLine(true);
  doc_text_.setReadOnly(true);
  doc_text_.setScrollbarsShown(true);
  doc_text_.setCaretVisible(false);
  doc_text_.setColour(juce::TextEditor::textColourId,
                      tempolink::juceapp::style::TextPrimary());
  doc_text_.setColour(juce::TextEditor::highlightColourId,
                      tempolink::juceapp::style::BorderStrong().withAlpha(0.35F));
  doc_text_.setColour(juce::TextEditor::highlightedTextColourId,
                      tempolink::juceapp::style::TextPrimary());
  doc_text_.setColour(juce::TextEditor::backgroundColourId,
                      tempolink::juceapp::style::PanelBackground());
  doc_text_.setColour(juce::TextEditor::outlineColourId,
                      tempolink::juceapp::style::BorderStrong());
  addAndMakeVisible(doc_text_);
}

void QnaView::setOnBack(std::function<void()> on_back) {
  on_back_ = std::move(on_back);
}

void QnaView::setOnReload(std::function<void()> on_reload) {
  on_reload_ = std::move(on_reload);
}

void QnaView::setStatusText(const juce::String& status_text) {
  status_label_.setText(status_text, juce::dontSendNotification);
}

void QnaView::setDocumentText(const juce::String& text) {
  doc_text_.setText(text, juce::dontSendNotification);
}

void QnaView::resized() {
  auto area = getLocalBounds().reduced(18);
  auto top = area.removeFromTop(60);
  back_button_.setBounds(top.removeFromLeft(120).reduced(0, 14));
  top.removeFromLeft(8);
  title_label_.setBounds(top.removeFromLeft(220));
  top.removeFromLeft(8);
  reload_button_.setBounds(top.removeFromLeft(100).reduced(0, 14));

  auto controls = area.removeFromTop(28);
  status_label_.setBounds(controls);
  area.removeFromTop(8);
  doc_text_.setBounds(area);
}

void QnaView::paint(juce::Graphics& g) {
  g.fillAll(tempolink::juceapp::style::LobbyBackground());
}
