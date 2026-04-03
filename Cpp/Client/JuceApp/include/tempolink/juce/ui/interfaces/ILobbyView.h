#pragma once

#include <string>
#include <vector>
#include <functional>

#include "tempolink/juce/ui/models/UiModels.h"
#include "tempolink/juce/network/http/room/RoomApiTypes.h"

class ILobbyView {
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

  virtual ~ILobbyView() = default;

  virtual void setRooms(const std::vector<RoomSummary>& rooms) = 0;
  virtual void setPreviewHandler(std::function<void(std::string)> on_preview_room) = 0;
  virtual void setEnterHandler(std::function<void(std::string)> on_enter_room) = 0;
  virtual void setCreateHandler(
      std::function<void(const std::string&, const RoomCreatePayload&)> on_create_room) = 0;
  virtual void setMyRoomsHandler(std::function<void()> on_open_my_rooms) = 0;
  virtual void setNavigationHandler(std::function<void(NavigationTarget)> on_navigation_selected) = 0;
  virtual void setFilterChangedHandler(std::function<void(const LobbyRoomFilter&)> on_filter_changed) = 0;
  virtual void setStatusText(const std::string& status_text) = 0;
};
