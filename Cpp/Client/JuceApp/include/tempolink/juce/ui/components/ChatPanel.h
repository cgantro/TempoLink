#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include <string>

#include "tempolink/juce/network/signaling/SignalingClient.h"

namespace tempolink::juceapp::ui {

class ChatPanel : public juce::Component,
                  public juce::TextEditor::Listener {
 public:
  struct Message {
    juce::String user_id;
    juce::String text;
    juce::String timestamp;
    bool is_local = false;
  };

  ChatPanel(SignalingClient& signaling_client);
  ~ChatPanel() override;

  void AddMessage(const Message& msg);
  void Clear();

  // Component overrides
  void paint(juce::Graphics& g) override;
  void resized() override;

  // TextEditor::Listener overrides
  void textEditorReturnKeyPressed(juce::TextEditor& editor) override;

 private:
  SignalingClient& signaling_client_;
  
  juce::TextEditor history_display_;
  juce::TextEditor input_field_;
  
  std::vector<Message> messages_;
  
  void SendCurrentMessage();
  void UpdateHistoryText();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChatPanel)
};

} // namespace tempolink::juceapp::ui
