#include "tempolink/audio/NullAudioCodec.h"

#include <algorithm>
#include <cassert>
#include <cstring>

namespace tempolink::audio {

bool NullAudioCodec::Initialize(std::uint32_t sample_rate_hz,
                                std::uint8_t channels,
                                std::uint16_t frame_size) {
  (void)sample_rate_hz;
  (void)frame_size;
  channels_ = std::max<std::uint8_t>(1, channels);
  return true;
}

std::vector<std::byte> NullAudioCodec::Encode(std::span<const std::int16_t> pcm) {
  std::vector<std::byte> encoded(pcm.size() * sizeof(std::int16_t));
  if (!pcm.empty()) {
    assert(encoded.data() != nullptr);
    assert(pcm.data() != nullptr);
    assert(encoded.size() == pcm.size() * sizeof(std::int16_t));
    std::memcpy(encoded.data(), pcm.data(), encoded.size());
  }
  return encoded;
}

std::vector<std::int16_t> NullAudioCodec::Decode(
    std::span<const std::byte> encoded) {
  if (encoded.size() % sizeof(std::int16_t) != 0) {
    return {};
  }
  std::vector<std::int16_t> pcm(encoded.size() / sizeof(std::int16_t));
  if (!pcm.empty()) {
    assert(pcm.data() != nullptr);
    assert(encoded.data() != nullptr);
    assert(pcm.size() * sizeof(std::int16_t) == encoded.size());
    std::memcpy(pcm.data(), encoded.data(), pcm.size() * sizeof(std::int16_t));
  }
  return pcm;
}

}  // namespace tempolink::audio
