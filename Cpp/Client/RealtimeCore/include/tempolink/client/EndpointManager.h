#pragma once

#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "tempolink/config/NetworkConstants.h"

namespace tempolink::client {

struct EndpointProfile {
  std::string profile_name = tempolink::config::kDefaultEndpointProfileName;
  std::string server_host = tempolink::config::kDefaultRelayHost;
  std::uint16_t server_port = tempolink::config::kDefaultRelayPort;
  std::uint32_t room_id = tempolink::config::kDefaultRoomId;
  std::uint32_t participant_id = tempolink::config::kDefaultParticipantId;
  std::string nickname = tempolink::config::kDefaultNickname;
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
