#pragma once

#include <atomic>
#include <span>

namespace tempolink::client::audio {

/// Computes smoothed level metering from audio buffers.
class LevelMeter {
 public:
  /// Update the meter with a new buffer of PCM samples.
  void Update(std::span<const float> pcm);

  /// Get the current smoothed level (0.0 — 1.0).
  float Level() const;

  /// Reset to zero.
  void Reset();

 private:
  std::atomic<float> level_{0.0F};
};

}  // namespace tempolink::client::audio
