#pragma once

#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace tempolink::client {

struct EndpointProfile {
  std::string profile_name = "default";
  std::string server_host;
  std::uint16_t server_port = 0;
  std::uint32_t room_id = 0;
  std::uint32_t participant_id = 0;
  std::string nickname = "guest";
};

class EndpointManager {
 public:
  bool AddOrUpdate(EndpointProfile profile);
  bool Remove(const std::string& profile_name);
  std::optional<EndpointProfile> Find(const std::string& profile_name) const;

  std::vector<std::string> ListProfileNames() const;
  void SetActiveProfile(const std::string& profile_name);
  std::optional<EndpointProfile> ActiveProfile() const;

 private:
  std::unordered_map<std::string, EndpointProfile> profiles_;
  std::string active_profile_name_;
  mutable std::mutex mutex_;
};

}  // namespace tempolink::client
