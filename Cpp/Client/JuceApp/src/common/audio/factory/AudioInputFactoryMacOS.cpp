#include "tempolink/audio/AudioInputFactory.h"

#include <memory>

#include "tempolink/audio/platform/macos/CoreAudioInputDevice.h"

namespace tempolink::audio {

std::unique_ptr<IAudioInputDevice> CreateDefaultAudioInputDevice() {
  return std::make_unique<CoreAudioInputDevice>();
}

}  // namespace tempolink::audio

