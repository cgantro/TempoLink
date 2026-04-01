#pragma once

#include <algorithm>
#include <array>
#include <cstdint>

namespace tempolink::config {

inline constexpr std::array<std::uint32_t, 6> kSupportedSampleRatesHz = {
    44100U, 48000U, 88200U, 96000U, 176400U, 192000U};

// Includes common studio-safe powers-of-two and currently used legacy values.
inline constexpr std::array<std::uint16_t, 13> kSupportedFrameSamples = {
    16U,  32U,  48U,   64U,   96U,  128U, 192U,
    240U, 256U, 480U,  512U,  960U, 1024U};

inline bool IsSupportedSampleRate(std::uint32_t sample_rate_hz) {
  return std::find(kSupportedSampleRatesHz.begin(),
                   kSupportedSampleRatesHz.end(),
                   sample_rate_hz) != kSupportedSampleRatesHz.end();
}

inline bool IsSupportedFrameSamples(std::uint16_t frame_samples) {
  return std::find(kSupportedFrameSamples.begin(),
                   kSupportedFrameSamples.end(),
                   frame_samples) != kSupportedFrameSamples.end();
}

}  // namespace tempolink::config

