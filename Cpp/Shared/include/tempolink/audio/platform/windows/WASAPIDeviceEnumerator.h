#pragma once

#include <vector>

#include "tempolink/audio/AudioDevice.h"

namespace tempolink::audio::platform::windows::detail {

std::vector<AudioDeviceInfo> EnumerateWasapiInputDevices();
std::vector<AudioDeviceInfo> EnumerateWasapiOutputDevices();

}  // namespace tempolink::audio::platform::windows::detail

