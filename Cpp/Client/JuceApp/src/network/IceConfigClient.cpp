#include "tempolink/juce/network/IceConfigClient.h"

#include <thread>
#include <utility>

#include <juce_events/juce_events.h>

namespace {

juce::URL buildUrl(const std::string& base_url, const std::string& path) {
  return juce::URL(juce::String(base_url + path));
}

bool isTurnUrl(const std::string& url) {
  return url.rfind("turn:", 0) == 0 || url.rfind("turns:", 0) == 0;
}

bool isStunUrl(const std::string& url) {
  return url.rfind("stun:", 0) == 0;
}

}  // namespace

IceConfigClient::IceConfigClient(std::string base_url)
    : base_url_(trimTrailingSlash(std::move(base_url))) {}

void IceConfigClient::fetchIceConfigAsync(Callback callback) const {
  std::thread([base_url = base_url_, callback = std::move(callback)]() mutable {
    juce::String error_text;
    IceConfigSnapshot snapshot;

    const juce::URL url = buildUrl(base_url, "/api/network/ice");
    int status = 0;
    auto stream = url.createInputStream(juce::URL::InputStreamOptions(
                                            juce::URL::ParameterHandling::inAddress)
                                            .withConnectionTimeoutMs(3500)
                                            .withStatusCode(&status)
                                            .withHttpRequestCmd("GET"));
    if (stream == nullptr) {
      error_text = "Failed to connect ICE API";
    } else {
      const juce::String body = stream->readEntireStreamAsString();
      if (status >= 200 && status < 300) {
        snapshot = parseIceConfig(body);
      } else {
        error_text = "ICE API returned HTTP " + juce::String(status);
      }
    }

    juce::MessageManager::callAsync(
        [callback = std::move(callback), snapshot = std::move(snapshot), error_text]() mutable {
          callback(error_text.isEmpty(), std::move(snapshot), error_text);
        });
  }).detach();
}

std::string IceConfigClient::trimTrailingSlash(std::string value) {
  while (!value.empty() && value.back() == '/') {
    value.pop_back();
  }
  return value;
}

IceConfigSnapshot IceConfigClient::parseIceConfig(const juce::String& json_text) {
  IceConfigSnapshot snapshot;

  const juce::var root = juce::JSON::parse(json_text);
  auto* obj = root.getDynamicObject();
  if (obj == nullptr) {
    return snapshot;
  }

  snapshot.credential_ttl_seconds =
      static_cast<long>(obj->getProperty("credentialTtlSeconds").toString().getLargeIntValue());

  const juce::var ice_servers = obj->getProperty("iceServers");
  if (!ice_servers.isArray()) {
    return snapshot;
  }
  const auto* arr = ice_servers.getArray();
  if (arr == nullptr) {
    return snapshot;
  }

  for (const auto& entry : *arr) {
    auto* server_obj = entry.getDynamicObject();
    if (server_obj == nullptr) {
      continue;
    }

    IceServerConfig config;
    config.username = server_obj->getProperty("username").toString().toStdString();
    config.credential = server_obj->getProperty("credential").toString().toStdString();

    const juce::var urls_var = server_obj->getProperty("urls");
    if (urls_var.isArray()) {
      if (const auto* urls_arr = urls_var.getArray(); urls_arr != nullptr) {
        for (const auto& item : *urls_arr) {
          const std::string url = item.toString().toStdString();
          if (url.empty()) {
            continue;
          }
          config.urls.push_back(url);
          if (isTurnUrl(url)) {
            ++snapshot.turn_server_count;
          }
          if (isStunUrl(url)) {
            ++snapshot.stun_server_count;
          }
        }
      }
    }

    snapshot.servers.push_back(std::move(config));
  }

  return snapshot;
}
