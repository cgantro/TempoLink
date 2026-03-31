#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace tempolink::server {

struct ParticipantEndpoint {
  std::uint32_t participant_id = 0;
  std::string host;
  std::uint16_t port = 0;
  std::chrono::steady_clock::time_point last_seen =
      std::chrono::steady_clock::now();
};

class RoomRegistry {
 public:
  void Upsert(std::uint32_t room_id, ParticipantEndpoint participant);
  void Remove(std::uint32_t room_id, std::uint32_t participant_id);

  std::vector<ParticipantEndpoint> GetPeers(std::uint32_t room_id,
                                            std::uint32_t exclude_id) const;
  std::size_t RoomCount() const;
  std::size_t ParticipantCount(std::uint32_t room_id) const;
  void RemoveStale(std::chrono::seconds max_idle);

 private:
  using ParticipantMap =
      std::unordered_map<std::uint32_t, ParticipantEndpoint>;
  std::unordered_map<std::uint32_t, ParticipantMap> rooms_;
  mutable std::mutex mutex_;
};

}  // namespace tempolink::server
