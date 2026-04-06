#pragma once

#include <memory>

#include "tempolink/audio/IAudioOutputDevice.h"

namespace tempolink::audio {

std::unique_ptr<IAudioOutputDevice> CreateDefaultAudioOutputDevice();

}  // namespace tempolink::audio

