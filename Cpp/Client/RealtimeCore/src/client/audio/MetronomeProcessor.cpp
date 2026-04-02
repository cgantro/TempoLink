#include "tempolink/client/audio/MetronomeProcessor.h"

#include <algorithm>
#include <cmath>

namespace tempolink::client::audio {

void MetronomeProcessor::Process(std::span<float> pcm, const AudioFormat& format) {
  if (!enabled_.load()) {
    return;
  }

  const int bpm = std::clamp(bpm_.load(), 30, 300);
  const std::uint64_t samples_per_beat =
      (static_cast<std::uint64_t>(format.sample_rate_hz) * 60ULL) /
      static_cast<std::uint64_t>(bpm);
  if (samples_per_beat == 0) {
    return;
  }

  const std::size_t channels = std::max<std::size_t>(1, format.channels);
  const std::size_t frame_samples = pcm.size() / channels;
  if (frame_samples == 0) {
    return;
  }

  const float metronome_gain = volume_.load();
  constexpr float kTwoPi = 6.28318530717958647692F;

  for (std::size_t sample_index = 0; sample_index < frame_samples;
       ++sample_index) {
    if (phase_samples_ == 0) {
      const bool accent = (beat_index_ % 4U) == 0U;
      const float accent_gain = accent ? 1.0F : 0.7F;
      const float frequency_hz = accent ? 1760.0F : 1320.0F;
      const std::size_t click_len =
          static_cast<std::size_t>(format.sample_rate_hz / 90U);

      for (std::size_t k = 0; k < click_len && sample_index + k < frame_samples;
           ++k) {
        const float t =
            static_cast<float>(k) / static_cast<float>(format.sample_rate_hz);
        const float envelope =
            1.0F - (static_cast<float>(k) / static_cast<float>(click_len));
        const float wave = std::sin(kTwoPi * frequency_hz * t);
        const float mixed =
            wave * envelope * accent_gain * metronome_gain * 0.5f;

        for (std::size_t ch = 0; ch < channels; ++ch) {
          const std::size_t index = (sample_index + k) * channels + ch;
          if (index >= pcm.size()) {
            continue;
          }
          pcm[index] += mixed;
        }
      }

      beat_index_ = (beat_index_ + 1U) % 4U;
    }
    phase_samples_ = (phase_samples_ + 1U) % samples_per_beat;
  }
}

void MetronomeProcessor::SetEnabled(bool enabled) { enabled_.store(enabled); }
bool MetronomeProcessor::IsEnabled() const { return enabled_.load(); }
void MetronomeProcessor::SetBpm(int bpm) { bpm_.store(bpm); }
int MetronomeProcessor::Bpm() const { return bpm_.load(); }
void MetronomeProcessor::SetVolume(float volume) { volume_.store(volume); }
float MetronomeProcessor::Volume() const { return volume_.load(); }

}  // namespace tempolink::client::audio
