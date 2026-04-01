#pragma once

#include <cstdint>
#include <span>

#include "tempolink/audio/IAudioInputDevice.h"
#include "tempolink/audio/IAudioOutputDevice.h"

namespace tempolink::client {

// Extension port for future DAW/VST bridge integration.
// Implementors can inspect/transform captured input and observe mixed output.
class AudioBridgePort {
 public:
  virtual ~AudioBridgePort() = default;

  virtual void OnCapturedInput(std::span<std::int16_t> frame,
                               const tempolink::audio::AudioCaptureConfig& config) = 0;

  virtual void OnPlaybackOutput(std::span<const std::int16_t> frame,
                                const tempolink::audio::AudioPlaybackConfig& config) = 0;
};

}  // namespace tempolink::client

