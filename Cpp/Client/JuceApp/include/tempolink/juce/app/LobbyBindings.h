#pragma once

#include <functional>
#include <string>

#include "tempolink/juce/ui/screens/LobbyView.h"
#include "tempolink/juce/ui/screens/MyRoomsView.h"
#include "tempolink/juce/ui/screens/RoomEntrySettingsView.h"

namespace tempolink::juceapp::app {

struct LobbyBindingsCallbacks {
  std::function<void(std::string)> on_preview_room;
  std::function<void(std::string)> on_enter_room;
  std::function<void(std::string, int)> on_create_room;
  std::function<void(const LobbyRoomFilter&)> on_filter_changed;
  std::function<void()> on_open_my_rooms;
  std::function<void(LobbyView::NavigationTarget)> on_navigation_selected;

  std::function<void()> on_my_rooms_back;
  std::function<void(std::string)> on_my_rooms_preview;
  std::function<void(std::string)> on_my_rooms_enter;
  std::function<void(std::string)> on_my_rooms_edit;
  std::function<void(std::string)> on_my_rooms_delete;
  std::function<void(std::string)> on_my_rooms_share;
  std::function<void(int)> on_my_rooms_create;
  std::function<void()> on_my_rooms_refresh;

  std::function<void()> on_entry_back;
  std::function<void(RoomEntrySettingsView::EntrySelection)> on_entry_join;
  std::function<void(std::string)> on_entry_part_changed;
  std::function<void(std::string)> on_entry_input_changed;
  std::function<void(std::string)> on_entry_output_changed;
  std::function<void()> on_entry_open_audio_settings;
};

void BindLobbyViews(LobbyView& lobby_view, MyRoomsView& my_rooms_view,
                    RoomEntrySettingsView& room_entry_view,
                    const LobbyBindingsCallbacks& callbacks);

}  // namespace tempolink::juceapp::app
