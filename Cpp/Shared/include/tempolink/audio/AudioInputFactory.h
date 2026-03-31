#pragma once

#include <memory>

#include "tempolink/audio/IAudioInputDevice.h"

namespace tempolink::audio {

std::unique_ptr<IAudioInputDevice> CreateDefaultAudioInputDevice();

}  // namespace tempolink::audio

