#include "tempolink/client/audio/InputGainProcessor.h"

#include <algorithm>
#include <cmath>

namespace tempolink::client::audio {

void InputGainProcessor::Process(std::span<float> pcm, const AudioFormat& /*format*/) {
  const float gain = gain_.load();
  if (std::abs(gain - 1.0F) < 0.0001F) {
    return;  // Unity gain — no work needed.
  }
  for (auto& sample : pcm) {
    sample *= gain;
  }
}

void InputGainProcessor::SetGain(float gain) {
  gain_.store(std::clamp(gain, 0.0F, 2.0F));
}

float InputGainProcessor::Gain() const { return gain_.load(); }

}  // namespace tempolink::client::audio
