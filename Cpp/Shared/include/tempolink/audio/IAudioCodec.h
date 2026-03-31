#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace tempolink::audio {

class IAudioCodec {
 public:
  virtual ~IAudioCodec() = default;

  virtual bool Initialize(std::uint32_t sample_rate_hz, std::uint8_t channels,
                          std::uint16_t frame_size) = 0;
  virtual std::vector<std::byte> Encode(std::span<const std::int16_t> pcm) = 0;
  virtual std::vector<std::int16_t> Decode(
      std::span<const std::byte> encoded) = 0;
};

}  // namespace tempolink::audio
