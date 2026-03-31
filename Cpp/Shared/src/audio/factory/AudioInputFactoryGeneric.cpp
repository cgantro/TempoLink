#include "tempolink/audio/AudioInputFactory.h"

#include <memory>

#include "tempolink/audio/platform/generic/NullAudioInputDevice.h"

namespace tempolink::audio {

std::unique_ptr<IAudioInputDevice> CreateDefaultAudioInputDevice() {
  return std::make_unique<NullAudioInputDevice>();
}

}  // namespace tempolink::audio

