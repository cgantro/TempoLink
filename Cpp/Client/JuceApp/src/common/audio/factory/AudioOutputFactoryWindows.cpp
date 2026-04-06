#include "tempolink/audio/AudioOutputFactory.h"

#include <memory>

#include "tempolink/audio/platform/windows/WASAPIAudioOutputDevice.h"

namespace tempolink::audio {

std::unique_ptr<IAudioOutputDevice> CreateDefaultAudioOutputDevice() {
  return std::make_unique<WASAPIAudioOutputDevice>();
}

}  // namespace tempolink::audio

