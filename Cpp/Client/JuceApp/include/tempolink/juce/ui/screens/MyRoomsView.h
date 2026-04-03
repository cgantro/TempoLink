#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>
#include "tempolink/juce/style/ThemeableComponent.h"
#include "tempolink/juce/ui/interfaces/IMyRoomsView.h"

#include "tempolink/juce/ui/components/RoomCardComponent.h"

class MyRoomsView final : public tempolink::juceapp::style::ThemeableComponent,
                          public IMyRoomsView {
 public:
  MyRoomsView();
  void updateTheme() override;

  void setRooms(const std::vector<RoomSummary>& rooms) override;
  void setStatusText(const std::string& status_text) override;

  void setPreviewHandler(std::function<void(std::string)> on_preview_room) override;
  void setEnterHandler(std::function<void(std::string)> on_enter_room) override;
  void setEditHandler(std::function<void(std::string)> on_edit_room) override;
  void setDeleteHandler(std::function<void(std::string)> on_delete_room) override;
  void setShareHandler(std::function<void(std::string)> on_share_room) override;
  void setCreateHandler(std::function<void(const RoomCreatePayload&)> on_create_room) override;
  void setRefreshHandler(std::function<void()> on_refresh) override;
  void setBackHandler(std::function<void()> on_back) override;

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
  std::function<void(const RoomCreatePayload&)> on_create_room_;
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
