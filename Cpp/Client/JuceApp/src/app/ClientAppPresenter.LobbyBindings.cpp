#include "tempolink/juce/app/ClientAppPresenter.h"

#include <utility>

#include "tempolink/juce/app/LobbyBindings.h"

void ClientAppPresenter::wireLobbyUiEvents() {
  using ScreenMode = tempolink::juceapp::app::ScreenMode;

  tempolink::juceapp::app::LobbyBindingsCallbacks lobby_callbacks;
  lobby_callbacks.on_preview_room =
      [this](std::string room_code) { previewRoom(room_code); };
  lobby_callbacks.on_enter_room =
      [this](std::string room_code) { openRoomEntry(room_code); };
  lobby_callbacks.on_create_room = [this](std::string host_user_id,
                                          int max_participants) {
    createRoomFromLobby(host_user_id, max_participants);
  };
  lobby_callbacks.on_filter_changed = [this](const LobbyRoomFilter& filter) {
    updateLobbyFilter(filter);
  };
  lobby_callbacks.on_open_my_rooms = [this] {
    my_rooms_view_.setStatusText(
        "My rooms: " +
        juce::String(static_cast<int>(lobby_data_controller_.OwnedRoomCount())));
    screen_navigator_.SwitchTo(ScreenMode::MyRooms);
  };
  lobby_callbacks.on_navigation_selected =
      [this](LobbyView::NavigationTarget target) { openNavigationScreen(target); };
  lobby_callbacks.on_my_rooms_back =
      [this] { screen_navigator_.SwitchTo(ScreenMode::Lobby); };
  lobby_callbacks.on_my_rooms_preview =
      [this](std::string room_code) { previewRoom(room_code); };
  lobby_callbacks.on_my_rooms_enter =
      [this](std::string room_code) { openRoomEntry(room_code); };
  lobby_callbacks.on_my_rooms_edit =
      [this](std::string room_code) { editRoomFromMyRooms(room_code); };
  lobby_callbacks.on_my_rooms_delete =
      [this](std::string room_code) { deleteRoomFromMyRooms(room_code); };
  lobby_callbacks.on_my_rooms_share =
      [this](std::string room_code) { shareRoomFromMyRooms(room_code); };
  lobby_callbacks.on_my_rooms_create =
      [this](int max_participants) { createRoomFromLobby("", max_participants); };
  lobby_callbacks.on_my_rooms_refresh = [this] { fetchRoomsFromApi(); };
  lobby_callbacks.on_entry_back = [this] {
    session_lifecycle_controller_.clear_entry_room_code();
    setLobbyStatusText("Entry cancelled.");
    screen_navigator_.SwitchTo(ScreenMode::Lobby);
  };
  lobby_callbacks.on_entry_join =
      [this](RoomEntrySettingsView::EntrySelection selection) {
        selected_part_label_ = selection.part_label;
        preferred_input_device_ = selection.input_device;
        preferred_output_device_ = selection.output_device;
        const std::string room_code =
            selection.room_code.empty()
                ? session_lifecycle_controller_.entry_room_code()
                : selection.room_code;
        if (room_code.empty()) {
          room_entry_view_.setStatusText("Room code missing.");
          return;
        }
        openRoom(room_code);
      };
  lobby_callbacks.on_entry_part_changed = [this](std::string part_label) {
    selected_part_label_ = std::move(part_label);
  };
  lobby_callbacks.on_entry_input_changed = [this](std::string input_device) {
    preferred_input_device_ = std::move(input_device);
  };
  lobby_callbacks.on_entry_output_changed = [this](std::string output_device) {
    preferred_output_device_ = std::move(output_device);
  };
  lobby_callbacks.on_entry_open_audio_settings = [this] {
    room_entry_view_.setStatusText("Advanced audio settings will open in-session.");
  };
  tempolink::juceapp::app::BindLobbyViews(lobby_view_, my_rooms_view_,
                                          room_entry_view_, lobby_callbacks);
}
