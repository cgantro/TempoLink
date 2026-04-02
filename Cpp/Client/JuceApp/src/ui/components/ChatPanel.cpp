#include "tempolink/juce/ui/components/ChatPanel.h"

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>

#include "tempolink/juce/logging/AppLogger.h"

namespace tempolink::juceapp::ui {

ChatPanel::ChatPanel(SignalingClient& signaling_client)
    : signaling_client_(signaling_client) {
  
  history_display_.setMultiLine(true);
  history_display_.setReadOnly(true);
  history_display_.setCaretVisible(false);
  history_display_.setScrollBarThickness(10);
  history_display_.setTextToShowWhenEmpty("Message history will appear here.", juce::Colours::grey);
  history_display_.setFont(juce::Font(14.0f));
  addAndMakeVisible(history_display_);

  input_field_.setMultiLine(false);
  input_field_.setReadOnly(false);
  input_field_.addListener(this);
  input_field_.setTextToShowWhenEmpty("Type a message and press Enter...", juce::Colours::grey);
  input_field_.setFont(juce::Font(14.0f));
  addAndMakeVisible(input_field_);
}

ChatPanel::~ChatPanel() {
  input_field_.removeListener(this);
}

void ChatPanel::AddMessage(const Message& msg) {
  messages_.push_back(msg);
  if (messages_.size() > 200) {
    messages_.erase(messages_.begin());
  }
  UpdateHistoryText();
}

void ChatPanel::Clear() {
  messages_.clear();
  UpdateHistoryText();
}

void ChatPanel::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::darkgrey.darker(0.5f));
  
  g.setColour(juce::Colours::grey);
  g.drawRect(getLocalBounds(), 1);
}

void ChatPanel::resized() {
  auto area = getLocalBounds().reduced(5);
  area.removeFromBottom(2); // spacing
  input_field_.setBounds(area.removeFromBottom(30));
  area.removeFromBottom(5); // spacing
  history_display_.setBounds(area);
}

void ChatPanel::textEditorReturnKeyPressed(juce::TextEditor& editor) {
  if (&editor == &input_field_) {
    SendCurrentMessage();
  }
}

void ChatPanel::SendCurrentMessage() {
  juce::String text = input_field_.getText().trim();
  if (text.isEmpty()) {
    return;
  }

  if (signaling_client_.isConnected()) {
    if (signaling_client_.sendChatMessage(text)) {
      Message local_msg;
      local_msg.user_id = "You";
      local_msg.text = text;
      local_msg.timestamp = juce::Time::getCurrentTime().toString(true, false);
      local_msg.is_local = true;
      AddMessage(local_msg);
    } else {
      tempolink::juceapp::logging::Error("Failed to send chat message.");
    }
  } else {
    tempolink::juceapp::logging::Error("Not connected to signaling server.");
  }

  input_field_.clear();
}

void ChatPanel::UpdateHistoryText() {
  juce::String history;
  for (const auto& msg : messages_) {
    history << "[" << msg.timestamp << "] ";
    history << msg.user_id << ": ";
    history << msg.text << "\n";
  }
  
  history_display_.setText(history, false);
  history_display_.moveCaretToEnd();
}

} // namespace tempolink::juceapp::ui
