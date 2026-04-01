#pragma once

#include "tempolink/config/AudioConstants.h"

namespace tempolink::juceapp::constants {

inline constexpr auto& kSupportedSampleRates =
    tempolink::config::kSupportedSampleRatesHz;
inline constexpr auto& kSupportedBufferSamples =
    tempolink::config::kSupportedFrameSamples;

}  // namespace tempolink::juceapp::constants
