#pragma once

#include <functional>
#include <string>
#include <vector>

#include <juce_core/juce_core.h>

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

class IceConfigClient {
 public:
  using Callback = std::function<void(bool, IceConfigSnapshot, juce::String)>;

  explicit IceConfigClient(std::string base_url);

  void fetchIceConfigAsync(Callback callback) const;

 private:
  static std::string trimTrailingSlash(std::string value);
  static IceConfigSnapshot parseIceConfig(const juce::String& json_text);

  std::string base_url_;
};
