#include "tempolink/juce/app/RoomCatalog.h"

#include <utility>

void RoomCatalog::clear() { rooms_.clear(); }

void RoomCatalog::setRooms(std::vector<RoomSummary> rooms) {
  rooms_ = std::move(rooms);
}

void RoomCatalog::upsertRoom(RoomSummary room) {
  for (auto& existing : rooms_) {
    if (existing.room_code == room.room_code) {
      existing = std::move(room);
      return;
    }
  }
  rooms_.push_back(std::move(room));
}

const RoomSummary* RoomCatalog::findByCode(const std::string& room_code) const {
  for (const auto& room : rooms_) {
    if (room.room_code == room_code) {
      return &room;
    }
  }
  return nullptr;
}

const std::vector<RoomSummary>& RoomCatalog::allRooms() const { return rooms_; }

std::vector<RoomSummary> RoomCatalog::ownedBy(
    const std::string& host_user_id) const {
  std::vector<RoomSummary> owned;
  owned.reserve(rooms_.size());
  for (const auto& room : rooms_) {
    if (room.host_user_id == host_user_id) {
      owned.push_back(room);
    }
  }
  return owned;
}

