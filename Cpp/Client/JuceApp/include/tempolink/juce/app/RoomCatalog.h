#pragma once

#include <string>
#include <vector>

#include "tempolink/juce/model/RoomSummary.h"

class RoomCatalog {
 public:
  void clear();
  void setRooms(std::vector<RoomSummary> rooms);
  void upsertRoom(RoomSummary room);

  const RoomSummary* findByCode(const std::string& room_code) const;
  const std::vector<RoomSummary>& allRooms() const;
  std::vector<RoomSummary> ownedBy(const std::string& host_user_id) const;

 private:
  std::vector<RoomSummary> rooms_;
};

