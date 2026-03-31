#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "tempolink/audio/IAudioCodec.h"

struct OpusDecoder;
struct OpusEncoder;

namespace tempolink::audio {

class OpusCodec final : public IAudioCodec {
 public:
  OpusCodec();
  ~OpusCodec() override;

  OpusCodec(const OpusCodec&) = delete;
  OpusCodec& operator=(const OpusCodec&) = delete;

  bool Initialize(std::uint32_t sample_rate_hz, std::uint8_t channels,
                  std::uint16_t frame_size) override;
  std::vector<std::byte> Encode(std::span<const std::int16_t> pcm) override;
  std::vector<std::int16_t> Decode(
      std::span<const std::byte> encoded) override;

 private:
  OpusEncoder* encoder_ = nullptr;
  OpusDecoder* decoder_ = nullptr;
  std::uint32_t sample_rate_hz_ = 48000;
  std::uint8_t channels_ = 1;
  std::uint16_t frame_size_ = 480;
};

}  // namespace tempolink::audio

