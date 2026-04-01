#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "tempolink/juce/ui/models/UiModels.h"

class ConnectionBadge final : public juce::Component {
 public:
  ConnectionBadge();

  void setState(ConnectionBadgeState state);
  void setText(const juce::String& text);

  void paint(juce::Graphics& g) override;

 private:
  ConnectionBadgeState state_ = ConnectionBadgeState::Unknown;
  juce::String text_override_;
};
