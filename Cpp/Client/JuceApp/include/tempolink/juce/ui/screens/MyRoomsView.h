#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>
#include "tempolink/juce/style/ThemeableComponent.h"
#include "tempolink/juce/ui/components/RoomCardComponent.h"

class MyRoomsView final : public tempolink::juceapp::style::ThemeableComponent {
 public:
  MyRoomsView();
  void updateTheme() override;

  void setRooms(const std::vector<RoomSummary>& rooms);
  void setStatusText(const juce::String& status_text);

  void setPreviewHandler(std::function<void(std::string)> on_preview_room);
  void setEnterHandler(std::function<void(std::string)> on_enter_room);
  void setEditHandler(std::function<void(std::string)> on_edit_room);
  void setDeleteHandler(std::function<void(std::string)> on_delete_room);
  void setShareHandler(std::function<void(std::string)> on_share_room);
  void setCreateHandler(std::function<void(int)> on_create_room);
  void setRefreshHandler(std::function<void()> on_refresh);
  void setBackHandler(std::function<void()> on_back);

  void resized() override;
  void paint(juce::Graphics& g) override;

 private:
  void rebuildRoomCards();
  void layoutRoomCards();

  std::vector<RoomSummary> rooms_;
  std::vector<std::unique_ptr<RoomCardComponent>> room_cards_;
  std::function<void(std::string)> on_preview_room_;
  std::function<void(std::string)> on_enter_room_;
  std::function<void(std::string)> on_edit_room_;
  std::function<void(std::string)> on_delete_room_;
  std::function<void(std::string)> on_share_room_;
  std::function<void(int)> on_create_room_;
  std::function<void()> on_refresh_;
  std::function<void()> on_back_;

  juce::Label title_label_;
  juce::Label status_label_;
  juce::TextButton back_button_{"Back to Rooms"};
  juce::TextButton create_button_{"Create Room"};
  juce::TextButton refresh_button_{"Refresh"};
  juce::Viewport cards_viewport_;
  juce::Component cards_container_;
};
