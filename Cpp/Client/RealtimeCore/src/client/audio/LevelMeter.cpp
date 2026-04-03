#include "tempolink/client/audio/LevelMeter.h"

#include <algorithm>
#include <cmath>

namespace tempolink::client::audio {

void LevelMeter::Update(std::span<const float> pcm) {
  if (pcm.empty()) {
    return;
  }

  // Compute normalized peak
  float max_abs = 0.0f;
  for (const auto sample : pcm) {
    const auto abs_sample = std::abs(sample);
    if (abs_sample > max_abs) {
      max_abs = abs_sample;
    }
  }
  const float peak = std::clamp(max_abs, 0.0F, 1.0F);

  // Smooth: fast attack, slow release
  constexpr float kAttack = 0.15F;
  constexpr float kRelease = 0.05F;
  const float previous = level_.load(std::memory_order_relaxed);
  const float coeff = (peak > previous) ? kAttack : kRelease;
  const float smoothed = previous + (peak - previous) * coeff;
  level_.store(smoothed, std::memory_order_relaxed);
}

float LevelMeter::Level() const {
  return level_.load(std::memory_order_relaxed);
}

void LevelMeter::Reset() {
  level_.store(0.0F, std::memory_order_relaxed);
}

}  // namespace tempolink::client::audio
