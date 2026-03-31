#pragma once

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "tempolink/juce/ui/common/PanelCard.h"
#include "tempolink/juce/ui/common/ConnectionBadge.h"

class TopStatusBar final : public juce::Component {
 public:
  TopStatusBar();

  void setRoomTitle(const juce::String& title);
  void setStatusText(const juce::String& status);
  void setConnectionMode(ConnectionBadgeState state);
  void setOnBack(std::function<void()> on_back);

  void resized() override;

 private:
  std::function<void()> on_back_;

  PanelCard card_;
  juce::Component body_;
  juce::TextButton back_button_{"Lobby"};
  juce::Label room_title_label_;
  ConnectionBadge connection_badge_;
  juce::Label status_label_;
};
