#pragma once

#include <string>

namespace tempolink::audio {

struct AudioDeviceInfo {
  std::string id;
  std::string name;
  bool is_default = false;
};

}  // namespace tempolink::audio

