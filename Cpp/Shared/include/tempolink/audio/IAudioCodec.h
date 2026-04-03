#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace tempolink::audio {

/// Abstract audio codec interface. All PCM data uses float [-1.0, 1.0].
/**
 * @brief Audio Codec 추상 클래스
 * @author 홍윤표
 * @details 현재는 Opus코덱을 기본으로 하지만, 추후 Codec 명세나 Codec이 늘어날 경우에 대비하여 인터페이스를 통한 추상화
 */
class IAudioCodec {
 public:
  virtual ~IAudioCodec() = default;

  virtual bool Initialize(std::uint32_t sample_rate_hz, std::uint8_t channels,
                          std::uint16_t frame_size) = 0;

  /// Encode float PCM → compressed bytes. Returns empty on failure.
  virtual std::vector<std::byte> Encode(std::span<const float> pcm) = 0;

  /// Decode compressed bytes → float PCM. Returns empty on failure.
  virtual std::vector<float> Decode(std::span<const std::byte> encoded) = 0;

  /// Optional: set bitrate (bps). Default implementation does nothing.
  virtual bool SetBitrate(int /*bitrate*/) { return false; }

  /// Optional: set complexity (0-10). Default implementation does nothing.
  virtual bool SetComplexity(int /*complexity*/) { return false; }
};

}  // namespace tempolink::audio
