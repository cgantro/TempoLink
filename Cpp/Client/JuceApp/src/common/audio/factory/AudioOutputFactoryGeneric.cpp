#include "tempolink/audio/AudioOutputFactory.h"

#include <memory>

#include "tempolink/audio/platform/generic/NullAudioOutputDevice.h"

namespace tempolink::audio {

std::unique_ptr<IAudioOutputDevice> CreateDefaultAudioOutputDevice() {
  return std::make_unique<NullAudioOutputDevice>();
}

}  // namespace tempolink::audio

