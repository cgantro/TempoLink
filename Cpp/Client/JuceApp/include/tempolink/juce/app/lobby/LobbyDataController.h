#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string>

#include "tempolink/juce/app/RoomCatalog.h"
#include "tempolink/juce/network/http/room/RoomApiClient.h"
#include "tempolink/juce/network/ice/IceConfigClient.h"
#include "tempolink/juce/ui/screens/LobbyView.h"
#include "tempolink/juce/ui/screens/MyRoomsView.h"

class LobbyDataController {
 public:
  LobbyDataController(RoomApiClient& room_api, IceConfigClient& ice_client,
                      RoomCatalog& room_catalog, LobbyView& lobby_view,
                      MyRoomsView& my_rooms_view,
                      std::shared_ptr<std::atomic_bool> alive_flag,
                      std::string& current_user_id_ref,
                      const std::function<void(const juce::String&)>&
                          set_lobby_status_text);

  void Tick(bool auto_refresh_enabled);
  void FetchRooms();
  void RefreshRoomViews();
  void FetchIceConfig();
  void UpdateFilter(const LobbyRoomFilter& filter);

  const RoomSummary* FindRoomByCode(const std::string& room_code) const;
  std::size_t OwnedRoomCount() const;
  const IceConfigSnapshot& ice_config() const;
  bool ice_config_loaded() const;

 private:
  RoomApiClient& room_api_;
  IceConfigClient& ice_client_;
  RoomCatalog& room_catalog_;
  LobbyView& lobby_view_;
  MyRoomsView& my_rooms_view_;
  std::shared_ptr<std::atomic_bool> alive_flag_;
  std::string& current_user_id_ref_;
  std::function<void(const juce::String&)> set_lobby_status_text_;

  int lobby_refresh_tick_ = 0;
  bool room_fetch_in_flight_ = false;
  bool ice_fetch_in_flight_ = false;
  LobbyRoomFilter lobby_filter_;
  IceConfigSnapshot ice_config_;
  bool ice_config_loaded_ = false;
};
