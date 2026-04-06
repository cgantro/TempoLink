#include "tempolink/audio/AudioInputFactory.h"

#include <memory>

#include "tempolink/audio/platform/linux/ALSAAudioInputDevice.h"

namespace tempolink::audio {

std::unique_ptr<IAudioInputDevice> CreateDefaultAudioInputDevice() {
  return std::make_unique<ALSAAudioInputDevice>();
}

}  // namespace tempolink::audio

