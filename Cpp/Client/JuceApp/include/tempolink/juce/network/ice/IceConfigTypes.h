#pragma once

#include <string>
#include <vector>

struct IceServerConfig {
  std::vector<std::string> urls;
  std::string username;
  std::string credential;
};

struct IceConfigSnapshot {
  std::vector<IceServerConfig> servers;
  long credential_ttl_seconds = 0;
  int stun_server_count = 0;
  int turn_server_count = 0;

  bool hasTurnFallback() const { return turn_server_count > 0; }
  bool hasDirectAssist() const { return stun_server_count > 0; }
};

