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
  lobby_callbacks.on_create_room = [this](const std::string& host_user_id,
                                          const RoomCreatePayload& payload) {
    createRoomFromLobby(host_user_id, payload);
  };
  lobby_callbacks.on_filter_changed = [this](const LobbyRoomFilter& filter) {
    updateLobbyFilter(filter);
  };
  lobby_callbacks.on_open_my_rooms = [this] {
    views_.my_rooms_view.setStatusText(
        "My rooms: " +
        std::to_string(lobby_data_controller_.OwnedRoomCount()));
    screen_navigator_.SwitchTo(ScreenMode::MyRooms);
  };
  lobby_callbacks.on_navigation_selected =
      [this](ILobbyView::NavigationTarget target) { openNavigationScreen(target); };
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
      [this](const RoomCreatePayload& payload) { createRoomFromLobby("", payload); };
  lobby_callbacks.on_my_rooms_refresh = [this] { fetchRoomsFromApi(); };
  lobby_callbacks.on_entry_back = [this] {
    session_lifecycle_controller_.clear_entry_room_code();
    setLobbyStatusText("Entry cancelled.");
    screen_navigator_.SwitchTo(ScreenMode::Lobby);
  };
  lobby_callbacks.on_entry_join =
      [this](IRoomEntryView::EntrySelection selection) {
        status_context_.selected_part_label = selection.part_label;
        status_context_.preferred_input_device = selection.input_device;
        status_context_.preferred_output_device = selection.output_device;
        const std::string room_code =
            selection.room_code.empty()
                ? session_lifecycle_controller_.entry_room_code()
                : selection.room_code;
        if (room_code.empty()) {
          views_.room_entry_view.setStatusText("Room code missing.");
          return;
        }
        openRoom(room_code);
      };
  lobby_callbacks.on_entry_part_changed = [this](std::string part_label) {
    status_context_.selected_part_label = std::move(part_label);
  };
  lobby_callbacks.on_entry_input_changed = [this](std::string input_device) {
    status_context_.preferred_input_device = std::move(input_device);
  };
  lobby_callbacks.on_entry_output_changed = [this](std::string output_device) {
    status_context_.preferred_output_device = std::move(output_device);
  };
  lobby_callbacks.on_entry_open_audio_settings = [this] {
    views_.room_entry_view.setStatusText("Advanced audio settings will open in-session.");
  };
  tempolink::juceapp::app::BindLobbyViews(views_.lobby_view, views_.my_rooms_view,
                                           views_.room_entry_view, lobby_callbacks);
}
