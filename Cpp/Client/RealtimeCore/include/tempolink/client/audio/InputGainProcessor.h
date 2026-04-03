#pragma once

#include <atomic>
#include <span>

#include "tempolink/client/audio/AudioProcessor.h"

namespace tempolink::client::audio {

/// Applies input gain to PCM in-place.
class InputGainProcessor final : public AudioProcessor {
 public:
  void Process(std::span<float> pcm, const AudioFormat& format) override;

  void SetGain(float gain);
  float Gain() const;

 private:
  std::atomic<float> gain_{1.0F};
};

}  // namespace tempolink::client::audio
