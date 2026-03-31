#include "tempolink/audio/AudioOutputFactory.h"

#include <memory>

#include "tempolink/audio/platform/macos/CoreAudioOutputDevice.h"

namespace tempolink::audio {

std::unique_ptr<IAudioOutputDevice> CreateDefaultAudioOutputDevice() {
  return std::make_unique<CoreAudioOutputDevice>();
}

}  // namespace tempolink::audio

