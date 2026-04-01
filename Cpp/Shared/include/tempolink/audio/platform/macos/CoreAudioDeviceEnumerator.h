#pragma once

#include <vector>

#include "tempolink/audio/AudioDevice.h"

namespace tempolink::audio::platform::macos::detail {

std::vector<AudioDeviceInfo> EnumerateCoreAudioInputDevices();
std::vector<AudioDeviceInfo> EnumerateCoreAudioOutputDevices();

}  // namespace tempolink::audio::platform::macos::detail

