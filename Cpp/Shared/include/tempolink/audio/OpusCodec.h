#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "tempolink/audio/IAudioCodec.h"

struct OpusDecoder;
struct OpusEncoder;

namespace tempolink::audio {

/// Opus codec implementation using float PCM and opus_encode_float/decode_float.
class OpusCodec final : public IAudioCodec {
 public:
  OpusCodec();
  ~OpusCodec() override;

  OpusCodec(const OpusCodec&) = delete;
  OpusCodec& operator=(const OpusCodec&) = delete;

  bool Initialize(std::uint32_t sample_rate_hz, std::uint8_t channels,
                  std::uint16_t frame_size) override;
  std::vector<std::byte> Encode(std::span<const float> pcm) override;
  std::vector<float> Decode(std::span<const std::byte> encoded) override;

  bool SetBitrate(int bitrate) override;
  bool SetComplexity(int complexity) override;

 private:
  OpusEncoder* encoder_ = nullptr;
  OpusDecoder* decoder_ = nullptr;
  std::uint32_t sample_rate_hz_ = 48000;
  std::uint8_t channels_ = 1;
  std::uint16_t frame_size_ = 480;
};

}  // namespace tempolink::audio
