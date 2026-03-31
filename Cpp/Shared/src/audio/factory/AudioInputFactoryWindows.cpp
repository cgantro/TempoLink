#include "tempolink/audio/AudioInputFactory.h"

#include <memory>

#include "tempolink/audio/platform/windows/WASAPIAudioInputDevice.h"

namespace tempolink::audio {

std::unique_ptr<IAudioInputDevice> CreateDefaultAudioInputDevice() {
  return std::make_unique<WASAPIAudioInputDevice>();
}

}  // namespace tempolink::audio

