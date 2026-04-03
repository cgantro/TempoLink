#pragma once

#include <string>
#include <vector>
#include <functional>

#include "tempolink/juce/ui/models/UiModels.h"
#include "tempolink/juce/network/http/room/RoomApiTypes.h"

class IMyRoomsView {
 public:
  virtual ~IMyRoomsView() = default;

  virtual void setRooms(const std::vector<RoomSummary>& rooms) = 0;
  virtual void setStatusText(const std::string& status_text) = 0;

  virtual void setPreviewHandler(std::function<void(std::string)> on_preview_room) = 0;
  virtual void setEnterHandler(std::function<void(std::string)> on_enter_room) = 0;
  virtual void setEditHandler(std::function<void(std::string)> on_edit_room) = 0;
  virtual void setDeleteHandler(std::function<void(std::string)> on_delete_room) = 0;
  virtual void setShareHandler(std::function<void(std::string)> on_share_room) = 0;
  virtual void setCreateHandler(std::function<void(const RoomCreatePayload&)> on_create_room) = 0;
  virtual void setRefreshHandler(std::function<void()> on_refresh) = 0;
  virtual void setBackHandler(std::function<void()> on_back) = 0;
};
