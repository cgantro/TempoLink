#include "tempolink/server/RoomRegistry.h"

#include <utility>

namespace tempolink::server {

void RoomRegistry::Upsert(std::uint32_t room_id, ParticipantEndpoint participant) {
  std::scoped_lock lock(mutex_);
  participant.last_seen = std::chrono::steady_clock::now();
  rooms_[room_id][participant.participant_id] = std::move(participant);
}

void RoomRegistry::Remove(std::uint32_t room_id, std::uint32_t participant_id) {
  std::scoped_lock lock(mutex_);
  auto room_it = rooms_.find(room_id);
  if (room_it == rooms_.end()) {
    return;
  }
  room_it->second.erase(participant_id);
  if (room_it->second.empty()) {
    rooms_.erase(room_it);
  }
}

std::vector<ParticipantEndpoint> RoomRegistry::GetPeers(std::uint32_t room_id,
                                                        std::uint32_t exclude_id) const {
  std::scoped_lock lock(mutex_);
  std::vector<ParticipantEndpoint> peers;
  auto room_it = rooms_.find(room_id);
  if (room_it == rooms_.end()) {
    return peers;
  }

  peers.reserve(room_it->second.size());
  for (const auto& [participant_id, participant] : room_it->second) {
    if (participant_id == exclude_id) {
      continue;
    }
    peers.push_back(participant);
  }

  return peers;
}

std::size_t RoomRegistry::RoomCount() const {
  std::scoped_lock lock(mutex_);
  return rooms_.size();
}

std::size_t RoomRegistry::ParticipantCount(std::uint32_t room_id) const {
  std::scoped_lock lock(mutex_);
  auto it = rooms_.find(room_id);
  return it == rooms_.end() ? 0 : it->second.size();
}

void RoomRegistry::RemoveStale(std::chrono::seconds max_idle) {
  std::scoped_lock lock(mutex_);
  const auto now = std::chrono::steady_clock::now();

  for (auto room_it = rooms_.begin(); room_it != rooms_.end();) {
    auto& participants = room_it->second;
    for (auto participant_it = participants.begin();
         participant_it != participants.end();) {
      const auto idle = now - participant_it->second.last_seen;
      if (idle > max_idle) {
        participant_it = participants.erase(participant_it);
      } else {
        ++participant_it;
      }
    }

    if (participants.empty()) {
      room_it = rooms_.erase(room_it);
    } else {
      ++room_it;
    }
  }
}

}  // namespace tempolink::server
