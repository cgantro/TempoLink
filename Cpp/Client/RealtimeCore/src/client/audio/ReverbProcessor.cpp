#include "tempolink/client/audio/ReverbProcessor.h"

#include <algorithm>
#include <mutex>

namespace tempolink::client::audio {

void ReverbProcessor::Process(std::span<float> pcm, const AudioFormat& format) {
  const float amount = amount_.load();
  if (amount <= 0.0001F || pcm.empty()) {
    return;
  }

  std::scoped_lock lock(mutex_);
  const std::size_t channels = std::max<std::size_t>(1, format.channels);
  const std::size_t expected_len =
      static_cast<std::size_t>(format.sample_rate_hz / 20U) * channels;

  if (delay_line_.size() != expected_len) {
    delay_line_.assign(std::max<std::size_t>(1, expected_len), 0.0f);
    write_index_ = 0;
  }

  const float wet = std::clamp(amount, 0.0F, 1.0F) * 0.55F;
  const float feedback = 0.30F + (wet * 0.35F);

  for (auto& sample : pcm) {
    const float delayed = delay_line_[write_index_];
    const float mixed = sample + (delayed * wet);
    sample = mixed;
    const float next_delay = (sample * 0.45F) + (delayed * feedback);
    delay_line_[write_index_] = next_delay;
    ++write_index_;
    if (write_index_ >= delay_line_.size()) {
      write_index_ = 0;
    }
  }
}

void ReverbProcessor::SetAmount(float amount) {
  amount_.store(std::clamp(amount, 0.0F, 1.0F));
}

float ReverbProcessor::Amount() const { return amount_.load(); }

void ReverbProcessor::Reset() {
  std::scoped_lock lock(mutex_);
  std::fill(delay_line_.begin(), delay_line_.end(), 0.0f);
  write_index_ = 0;
}

}  // namespace tempolink::client::audio
