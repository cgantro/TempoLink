#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "tempolink/audio/IAudioCodec.h"

namespace tempolink::audio {

/// Pass-through codec: wraps float PCM as raw bytes with no compression.
class NullAudioCodec final : public IAudioCodec {
 public:
  bool Initialize(std::uint32_t sample_rate_hz, std::uint8_t channels,
                  std::uint16_t frame_size) override;
  std::vector<std::byte> Encode(std::span<const float> pcm) override;
  std::vector<float> Decode(std::span<const std::byte> encoded) override;

 private:
  std::uint8_t channels_ = 1;
};

}  // namespace tempolink::audio
