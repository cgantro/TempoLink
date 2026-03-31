#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "tempolink/audio/IAudioCodec.h"

namespace tempolink::audio {

class NullAudioCodec final : public IAudioCodec {
 public:
  bool Initialize(std::uint32_t sample_rate_hz, std::uint8_t channels,
                  std::uint16_t frame_size) override;
  std::vector<std::byte> Encode(std::span<const std::int16_t> pcm) override;
  std::vector<std::int16_t> Decode(
      std::span<const std::byte> encoded) override;

 private:
  std::uint8_t channels_ = 1;
};

}  // namespace tempolink::audio

