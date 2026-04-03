#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>
#include "tempolink/juce/style/ThemeableComponent.h"
#include "tempolink/juce/ui/interfaces/ILobbyView.h"

#include "tempolink/juce/ui/components/RoomCardComponent.h"

class LobbyView final : public tempolink::juceapp::style::ThemeableComponent,
                        public ILobbyView {
 public:
  LobbyView();
  void updateTheme() override;

  void setRooms(const std::vector<RoomSummary>& rooms) override;
  void setLogoImage(const juce::Image& logo_image);
  void setPreviewHandler(std::function<void(std::string)> on_preview_room) override;
  void setEnterHandler(std::function<void(std::string)> on_enter_room) override;
  void setCreateHandler(
      std::function<void(const std::string&, const RoomCreatePayload&)> on_create_room) override;
  void setMyRoomsHandler(std::function<void()> on_open_my_rooms) override;
  void setNavigationHandler(
      std::function<void(NavigationTarget)> on_navigation_selected) override;
  void setFilterChangedHandler(std::function<void(const LobbyRoomFilter&)> on_filter_changed) override;
  void setStatusText(const std::string& status_text) override;

  void resized() override;
  void paint(juce::Graphics& g) override;

 private:
  void rebuildRoomCards();
  void layoutRoomCards();

  std::vector<RoomSummary> rooms_;
  std::vector<std::unique_ptr<RoomCardComponent>> room_cards_;
  std::function<void(std::string)> on_preview_room_;
  std::function<void(std::string)> on_enter_room_;
  std::function<void(const std::string&, const RoomCreatePayload&)> on_create_room_;
  std::function<void()> on_open_my_rooms_;
  std::function<void(NavigationTarget)> on_navigation_selected_;
  std::function<void(const LobbyRoomFilter&)> on_filter_changed_;
  std::vector<std::unique_ptr<juce::TextButton>> nav_buttons_;
  LobbyRoomFilter filter_;

  juce::Label title_label_;
  juce::Label tabs_label_;
  juce::Label status_label_;
  juce::ImageComponent logo_image_;
  juce::Label logo_text_label_;
  juce::Label nav_title_label_;
  juce::TextEditor search_editor_;
  juce::TextEditor tag_filter_editor_;
  juce::ToggleButton active_filter_toggle_{"Active Rooms"};
  juce::ToggleButton standby_filter_toggle_{"Standby Rooms"};
  juce::ComboBox visibility_filter_combo_;
  juce::ComboBox password_filter_combo_;
  juce::TextButton room_action_fab_{"+"};
  juce::Viewport cards_viewport_;
  juce::Component cards_container_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LobbyView)
};
