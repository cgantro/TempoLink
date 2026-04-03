#pragma once

#include <string>
#include <cstdint>

namespace tempolink::juceapp::app {

/**
 * Parameter Object grouping shared application state and configuration.
 * Used to reduce constructor parameter explosion in Controllers.
 */
struct AppStatusContext {
  // Session State
  bool auth_completed = false;
  std::string current_user_id;
  std::string current_display_name;
  
  // Audio Preferences
  std::string selected_part_label;
  std::string preferred_input_device;
  std::string preferred_output_device;

  // Network Configuration
  std::string control_plane_host;
  std::uint16_t control_plane_port = 0;
  std::string default_relay_host;
  std::uint16_t default_relay_port = 0;
};

} // namespace tempolink::juceapp::app
