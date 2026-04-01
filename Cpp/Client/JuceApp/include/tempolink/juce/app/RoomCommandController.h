#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string>

#include <juce_core/juce_core.h>

#include "tempolink/juce/app/RoomCatalog.h"
#include "tempolink/juce/network/http/room/RoomApiClient.h"
#include "tempolink/juce/ui/screens/MyRoomsView.h"

class RoomCommandController {
 public:
  RoomCommandController(RoomApiClient& room_api, RoomCatalog& room_catalog,
                        MyRoomsView& my_rooms_view,
                        std::shared_ptr<std::atomic_bool> alive_flag,
                        std::string& current_user_id_ref);

  void setCallbacks(std::function<void()> refresh_room_views,
                    std::function<void()> fetch_rooms_from_api,
                    std::function<void(const juce::String&)> set_lobby_status_text,
                    std::function<void(const std::string&)> open_room_entry);

  void createRoom(const std::string& host_user_id, int max_participants);
  void editRoom(const std::string& room_code);
  void deleteRoom(const std::string& room_code);
  void shareRoom(const std::string& room_code);

 private:
  RoomApiClient& room_api_;
  RoomCatalog& room_catalog_;
  MyRoomsView& my_rooms_view_;
  std::shared_ptr<std::atomic_bool> alive_flag_;
  std::string& current_user_id_ref_;

  std::function<void()> refresh_room_views_;
  std::function<void()> fetch_rooms_from_api_;
  std::function<void(const juce::String&)> set_lobby_status_text_;
  std::function<void(const std::string&)> open_room_entry_;
};

