#pragma once

#include <cstdint>
#include <string>

namespace tempolink::juceapp::config {

struct ClientEnvConfig {
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

  static ClientEnvConfig Load();
};

}  // namespace tempolink::juceapp::config
