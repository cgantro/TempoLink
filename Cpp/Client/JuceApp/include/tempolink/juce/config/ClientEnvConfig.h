#pragma once

#include <cstdint>
#include <string>

namespace tempolink::juceapp::config {

struct ClientEnvConfig {
  static constexpr const char* kDefaultControlPlaneBaseUrl = "http://127.0.0.1:8080";
  static constexpr const char* kDefaultControlPlaneHost = "127.0.0.1";
  static constexpr std::uint16_t kDefaultControlPlanePort = 8080;

  static constexpr const char* kDefaultRelayHost = "127.0.0.1";
  static constexpr std::uint16_t kDefaultRelayPort = 40000;

  static constexpr std::uint32_t kDefaultRoomId = 1;
  static constexpr std::uint32_t kDefaultParticipantId = 1001;
  static constexpr const char* kDefaultNickname = "guest";
  static constexpr const char* kDefaultEndpointProfileName = "default";

  std::string control_plane_base_url;
  std::string control_plane_host;
  std::uint16_t control_plane_port = 0;

  std::string default_relay_host;
  std::uint16_t default_relay_port = 0;

  std::string oauth_callback_host;
  std::uint16_t oauth_callback_port = 0;
  std::string oauth_callback_path;

  std::string bridge_host;
  std::uint16_t bridge_client_listen_port = 0;
  std::uint16_t bridge_plugin_listen_port = 0;

  static void SetEnvironment(const std::string& env);
  static std::string GetEnvironment();
  static ClientEnvConfig Load();
};

}  // namespace tempolink::juceapp::config
