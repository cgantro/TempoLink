#pragma once

#include <memory>

#include "tempolink/audio/IAudioCodec.h"

namespace tempolink::audio {

std::unique_ptr<IAudioCodec> CreateDefaultAudioCodec();

}  // namespace tempolink::audio

