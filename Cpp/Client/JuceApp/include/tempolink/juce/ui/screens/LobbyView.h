#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "tempolink/juce/ui/components/RoomCardComponent.h"

struct LobbyRoomFilter {
  std::string query;
  std::string tag;
  std::optional<bool> is_public;
  std::optional<bool> has_password;
  std::string mode = "all";
};

class LobbyView final : public juce::Component {
 public:
  enum class NavigationTarget {
    Rooms,
    Profile,
    Users,
    News,
    Manual,
    Qna,
    Settings
  };

  LobbyView();

  void setRooms(const std::vector<RoomSummary>& rooms);
  void setLogoImage(const juce::Image& logo_image);
  void setPreviewHandler(std::function<void(std::string)> on_preview_room);
  void setEnterHandler(std::function<void(std::string)> on_enter_room);
  void setCreateHandler(std::function<void(std::string, int)> on_create_room);
  void setMyRoomsHandler(std::function<void()> on_open_my_rooms);
  void setNavigationHandler(
      std::function<void(NavigationTarget)> on_navigation_selected);
  void setFilterChangedHandler(std::function<void(const LobbyRoomFilter&)> on_filter_changed);
  void setStatusText(const juce::String& status_text);

  void resized() override;
  void paint(juce::Graphics& g) override;

 private:
  void rebuildRoomCards();
  void layoutRoomCards();

  std::vector<RoomSummary> rooms_;
  std::vector<std::unique_ptr<RoomCardComponent>> room_cards_;
  std::function<void(std::string)> on_preview_room_;
  std::function<void(std::string)> on_enter_room_;
  std::function<void(std::string, int)> on_create_room_;
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
};
