#include "tempolink/juce/app/lobby/LobbyDataController.h"

#include <utility>
#include <vector>

#include "tempolink/juce/constants/ClientText.h"
#include "tempolink/juce/style/UiStyle.h"

LobbyDataController::LobbyDataController(
    RoomApiClient& room_api, IceConfigClient& ice_client,
    RoomCatalog& room_catalog, LobbyView& lobby_view, MyRoomsView& my_rooms_view,
    std::shared_ptr<std::atomic_bool> alive_flag,
    std::string& current_user_id_ref,
    const std::function<void(const juce::String&)>& set_lobby_status_text)
    : room_api_(room_api),
      ice_client_(ice_client),
      room_catalog_(room_catalog),
      lobby_view_(lobby_view),
      my_rooms_view_(my_rooms_view),
      alive_flag_(std::move(alive_flag)),
      current_user_id_ref_(current_user_id_ref),
      set_lobby_status_text_(set_lobby_status_text) {}

void LobbyDataController::Tick(bool auto_refresh_enabled) {
  if (!auto_refresh_enabled) {
    return;
  }

  ++lobby_refresh_tick_;
  if (!room_fetch_in_flight_ &&
      lobby_refresh_tick_ >= tempolink::juceapp::style::kLobbyRefreshTicks) {
    lobby_refresh_tick_ = 0;
    FetchRooms();
  }
}

void LobbyDataController::FetchRooms() {
  if (room_fetch_in_flight_) {
    return;
  }

  room_fetch_in_flight_ = true;
  RoomListFilter api_filter;
  api_filter.query = lobby_filter_.query;
  api_filter.tag = lobby_filter_.tag;
  api_filter.is_public = lobby_filter_.is_public;
  api_filter.has_password = lobby_filter_.has_password;
  api_filter.mode = lobby_filter_.mode;

  auto alive = alive_flag_;
  room_api_.fetchRoomsAsync(
      api_filter,
      [this, alive](bool ok, std::vector<RoomSummary> rooms,
                    juce::String error_text) {
        if (!alive->load()) {
          return;
        }
        room_fetch_in_flight_ = false;
        if (!ok) {
          room_catalog_.clear();
          RefreshRoomViews();
          set_lobby_status_text_(
              juce::String(tempolink::juceapp::text::kRoomApiUnavailablePrefix) +
              error_text + tempolink::juceapp::text::kShowingMockRoomsSuffix);
          my_rooms_view_.setStatusText("Failed to load MY ROOMS: " + error_text);
          return;
        }

        if (rooms.empty()) {
          room_catalog_.clear();
          RefreshRoomViews();
          set_lobby_status_text_(tempolink::juceapp::text::kRoomApiEmptyFallback);
          my_rooms_view_.setStatusText("No rooms created yet.");
          return;
        }

        room_catalog_.setRooms(std::move(rooms));
        const int room_count = static_cast<int>(room_catalog_.allRooms().size());
        RefreshRoomViews();
        set_lobby_status_text_(juce::String(tempolink::juceapp::text::kLoadedRoomsPrefix) +
                               juce::String(room_count) +
                               tempolink::juceapp::text::kLoadedRoomsUserDelimiter +
                               juce::String(current_user_id_ref_));
        my_rooms_view_.setStatusText("My rooms: " +
                                     juce::String(static_cast<int>(
                                         room_catalog_.ownedBy(current_user_id_ref_)
                                             .size())));
      });
}

void LobbyDataController::RefreshRoomViews() {
  lobby_view_.setRooms(room_catalog_.allRooms());
  my_rooms_view_.setRooms(room_catalog_.ownedBy(current_user_id_ref_));
}

void LobbyDataController::FetchIceConfig() {
  if (ice_fetch_in_flight_) {
    return;
  }

  ice_fetch_in_flight_ = true;
  auto alive = alive_flag_;
  ice_client_.fetchIceConfigAsync(
      [this, alive](bool ok, IceConfigSnapshot snapshot, juce::String error_text) {
        if (!alive->load()) {
          return;
        }
        ice_fetch_in_flight_ = false;
        if (!ok) {
          set_lobby_status_text_(
              juce::String(tempolink::juceapp::text::kIceConfigUnavailablePrefix) +
              error_text);
          return;
        }

        ice_config_ = std::move(snapshot);
        ice_config_loaded_ = true;
        const juce::String direct_hint = ice_config_.hasDirectAssist()
                                             ? tempolink::juceapp::text::kP2PAssistReady
                                             : tempolink::juceapp::text::kP2PAssistLimited;
        set_lobby_status_text_(
            juce::String(tempolink::juceapp::text::kNetworkProfileLoadedPrefix) +
            direct_hint +
            tempolink::juceapp::text::kNetworkProfileLoadedSuffix);
      });
}

void LobbyDataController::UpdateFilter(const LobbyRoomFilter& filter) {
  lobby_filter_ = filter;
  FetchRooms();
}

const RoomSummary* LobbyDataController::FindRoomByCode(
    const std::string& room_code) const {
  return room_catalog_.findByCode(room_code);
}

std::size_t LobbyDataController::OwnedRoomCount() const {
  return room_catalog_.ownedBy(current_user_id_ref_).size();
}

const IceConfigSnapshot& LobbyDataController::ice_config() const {
  return ice_config_;
}

bool LobbyDataController::ice_config_loaded() const { return ice_config_loaded_; }
