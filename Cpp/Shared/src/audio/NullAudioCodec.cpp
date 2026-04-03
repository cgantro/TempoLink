#include "tempolink/audio/NullAudioCodec.h"

#include <cstring>

namespace tempolink::audio {

bool NullAudioCodec::Initialize(std::uint32_t /*sample_rate_hz*/,
                                std::uint8_t channels,
                                std::uint16_t /*frame_size*/) {
  channels_ = channels;
  return true;
}

std::vector<std::byte> NullAudioCodec::Encode(std::span<const float> pcm) {
  // Wrap raw float samples as bytes — no compression.
  const auto byte_size = pcm.size_bytes();
  std::vector<std::byte> out(byte_size);
  std::memcpy(out.data(), pcm.data(), byte_size);
  return out;
}

std::vector<float> NullAudioCodec::Decode(std::span<const std::byte> encoded) {
  const auto float_count = encoded.size() / sizeof(float);
  std::vector<float> out(float_count);
  std::memcpy(out.data(), encoded.data(), float_count * sizeof(float));
  return out;
}

}  // namespace tempolink::audio
