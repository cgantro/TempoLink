#pragma once

#include <functional>
#include <string>

#include <juce_gui_basics/juce_gui_basics.h>

#include "tempolink/juce/ui/UiModels.h"

class RoomCardComponent final : public juce::Component {
 public:
  RoomCardComponent();

  void setRoom(const RoomSummary& room);
  void setJoinHandler(std::function<void(std::string)> on_join);

  void resized() override;
  void paint(juce::Graphics& g) override;

 private:
  RoomSummary room_;
  std::function<void(std::string)> on_join_;

  juce::Label title_label_;
  juce::Label meta_label_;
  juce::Label host_label_;
  juce::Label latency_label_;
  juce::TextButton join_button_{"Join"};
};
