#include "tempolink/audio/AudioOutputFactory.h"

#include <memory>

#include "tempolink/audio/platform/linux/ALSAAudioOutputDevice.h"

namespace tempolink::audio {

std::unique_ptr<IAudioOutputDevice> CreateDefaultAudioOutputDevice() {
  return std::make_unique<ALSAAudioOutputDevice>();
}

}  // namespace tempolink::audio

