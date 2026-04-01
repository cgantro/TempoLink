#include "tempolink/juce/app/LobbyBindings.h"

#include <utility>

namespace tempolink::juceapp::app {

void BindLobbyViews(LobbyView& lobby_view, MyRoomsView& my_rooms_view,
                    RoomEntrySettingsView& room_entry_view,
                    const LobbyBindingsCallbacks& callbacks) {
  lobby_view.setPreviewHandler(callbacks.on_preview_room);
  lobby_view.setEnterHandler(callbacks.on_enter_room);
  lobby_view.setCreateHandler(callbacks.on_create_room);
  lobby_view.setFilterChangedHandler(callbacks.on_filter_changed);
  lobby_view.setMyRoomsHandler(callbacks.on_open_my_rooms);
  lobby_view.setNavigationHandler(callbacks.on_navigation_selected);

  my_rooms_view.setBackHandler(callbacks.on_my_rooms_back);
  my_rooms_view.setPreviewHandler(callbacks.on_my_rooms_preview);
  my_rooms_view.setEnterHandler(callbacks.on_my_rooms_enter);
  my_rooms_view.setEditHandler(callbacks.on_my_rooms_edit);
  my_rooms_view.setDeleteHandler(callbacks.on_my_rooms_delete);
  my_rooms_view.setShareHandler(callbacks.on_my_rooms_share);
  my_rooms_view.setCreateHandler(callbacks.on_my_rooms_create);
  my_rooms_view.setRefreshHandler(callbacks.on_my_rooms_refresh);

  room_entry_view.setOnBack(callbacks.on_entry_back);
  room_entry_view.setOnJoin(callbacks.on_entry_join);
  room_entry_view.setOnPartChanged(callbacks.on_entry_part_changed);
  room_entry_view.setOnInputDeviceChanged(callbacks.on_entry_input_changed);
  room_entry_view.setOnOutputDeviceChanged(callbacks.on_entry_output_changed);
  room_entry_view.setOnOpenAudioSettings(callbacks.on_entry_open_audio_settings);
}

}  // namespace tempolink::juceapp::app
