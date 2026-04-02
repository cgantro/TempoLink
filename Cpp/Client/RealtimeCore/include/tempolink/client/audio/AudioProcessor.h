#pragma once

#include <cstdint>
#include <span>

namespace tempolink::client::audio {

struct AudioFormat {
  std::uint32_t sample_rate_hz = 48000;
  std::uint8_t channels = 1;
  std::uint16_t frame_samples = 480;
};

/**
 * @brief Base interface for all audio processing and signal generation components.
 */
class AudioProcessor {
 public:
  virtual ~AudioProcessor() = default;

  /**
   * @brief Process a block of PCM data in-place.
   * @param pcm The audio buffer to process.
   * @param format The format of the audio data.
   */
  virtual void Process(std::span<float> pcm, const AudioFormat& format) = 0;
};

}  // namespace tempolink::client::audio
