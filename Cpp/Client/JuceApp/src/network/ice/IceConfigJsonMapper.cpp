#include "tempolink/juce/network/ice/IceConfigJsonMapper.h"

#include <nlohmann/json.hpp>

#include "tempolink/juce/network/http/common/JsonParse.h"

namespace tempolink::juceapp::network::iceconfig {

namespace {

using nlohmann::json;
namespace jp = tempolink::juceapp::jsonparse;

bool IsTurnUrl(const std::string& url) {
  return url.rfind("turn:", 0) == 0 || url.rfind("turns:", 0) == 0;
}

bool IsStunUrl(const std::string& url) { return url.rfind("stun:", 0) == 0; }

}  // namespace

IceConfigSnapshot ParseIceConfig(const juce::String& json_text) {
  IceConfigSnapshot snapshot;

  const json root = jp::Parse(json_text);
  if (root.is_discarded() || !root.is_object()) {
    return snapshot;
  }

  snapshot.credential_ttl_seconds = jp::Long(root, "credentialTtlSeconds", 0L);
  if (!root.contains("iceServers") || !root["iceServers"].is_array()) {
    return snapshot;
  }
  for (const auto& entry : root["iceServers"]) {
    if (!entry.is_object()) {
      continue;
    }

    IceServerConfig config;
    config.username = jp::String(entry, "username", "");
    config.credential = jp::String(entry, "credential", "");
    if (entry.contains("urls") && entry["urls"].is_array()) {
      for (const auto& item : entry["urls"]) {
        if (!item.is_string()) {
          continue;
        }
        const std::string url = item.get<std::string>();
        if (url.empty()) {
          continue;
        }
        config.urls.push_back(url);
        if (IsTurnUrl(url)) {
          ++snapshot.turn_server_count;
        }
        if (IsStunUrl(url)) {
          ++snapshot.stun_server_count;
        }
      }
    }

    snapshot.servers.push_back(std::move(config));
  }

  return snapshot;
}

}  // namespace tempolink::juceapp::network::iceconfig

