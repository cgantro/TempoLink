#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <vector>

#include "tempolink/client/audio/AudioProcessor.h"

namespace tempolink::client::audio {

class ReverbProcessor final : public AudioProcessor {
 public:
  void Process(std::span<float> pcm, const AudioFormat& format) override;

  void SetAmount(float amount);
  float Amount() const;
  void Reset();

 private:
  std::atomic<float> amount_{0.0F};
  std::vector<float> delay_line_;
  std::size_t write_index_ = 0;
  mutable std::mutex mutex_;
};

}  // namespace tempolink::client::audio
