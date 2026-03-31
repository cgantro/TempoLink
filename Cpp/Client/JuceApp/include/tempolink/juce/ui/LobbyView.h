#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "tempolink/juce/ui/RoomCardComponent.h"

class LobbyView final : public juce::Component {
 public:
  LobbyView();

  void setRooms(const std::vector<RoomSummary>& rooms);
  void setJoinHandler(std::function<void(std::string)> on_join_room);
  void setStatusText(const juce::String& status_text);

  void resized() override;
  void paint(juce::Graphics& g) override;

 private:
  void rebuildRoomCards();
  void layoutRoomCards();

  std::vector<RoomSummary> rooms_;
  std::vector<std::unique_ptr<RoomCardComponent>> room_cards_;
  std::function<void(std::string)> on_join_room_;

  juce::Label title_label_;
  juce::Label status_label_;
  juce::TextEditor search_editor_;
  juce::Viewport cards_viewport_;
  juce::Component cards_container_;
};
