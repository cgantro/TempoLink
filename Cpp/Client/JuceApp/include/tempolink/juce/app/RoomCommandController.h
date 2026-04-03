#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <optional>
#include <string>

#include <juce_core/juce_core.h>

#include "tempolink/juce/app/RoomCatalog.h"
#include "tempolink/juce/network/http/room/RoomApiClient.h"

class RoomCommandController {
 public:
  struct Dependencies {
    RoomApiClient& room_api;
    RoomCatalog& room_catalog;
    std::shared_ptr<std::atomic_bool> alive_flag;
    std::string& current_user_id_ref;
  };

  struct Callbacks {
    std::function<void()> refresh_room_views;
    std::function<void()> fetch_rooms_from_api;
    std::function<void(const juce::String&)> set_lobby_status_text;
    std::function<void(const std::string&)> open_room_entry;
    std::function<void(const std::string&)> set_my_rooms_status_text;
    std::function<void(const RoomSummary&,
                       std::function<void(std::optional<RoomUpdatePayload>)>)>
        request_room_edit_payload;
    std::function<void(const std::string&, std::function<void(bool)>)>
        request_delete_confirmation;
    std::function<void(const juce::String&)> copy_to_clipboard;
  };

  RoomCommandController(Dependencies dependencies, Callbacks callbacks);

  void createRoom(const std::string& host_user_id, int max_participants);
  void createRoom(const std::string& host_user_id, const RoomCreatePayload& payload);
  void editRoom(const std::string& room_code);
  void deleteRoom(const std::string& room_code);
  void shareRoom(const std::string& room_code);

 private:
  RoomApiClient& room_api_;
  RoomCatalog& room_catalog_;
  std::shared_ptr<std::atomic_bool> alive_flag_;
  std::string& current_user_id_ref_;
  Callbacks callbacks_;
};
