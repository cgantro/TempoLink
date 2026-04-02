#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>

#include "tempolink/client/audio/AudioProcessor.h"

namespace tempolink::client::audio {

class MetronomeProcessor final : public AudioProcessor {
 public:
  void Process(std::span<float> pcm, const AudioFormat& format) override;

  void SetEnabled(bool enabled);
  bool IsEnabled() const;
  void SetBpm(int bpm);
  int Bpm() const;
  void SetVolume(float volume);
  float Volume() const;

 private:
  std::atomic_bool enabled_{false};
  std::atomic<int> bpm_{120};
  std::atomic<float> volume_{0.35F};
  std::uint64_t phase_samples_ = 0;
  std::uint32_t beat_index_ = 0;
};

}  // namespace tempolink::client::audio
