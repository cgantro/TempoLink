#pragma once

#include <juce_core/juce_core.h>

#include "tempolink/juce/network/ice/IceConfigTypes.h"

namespace tempolink::juceapp::network::iceconfig {

IceConfigSnapshot ParseIceConfig(const juce::String& json_text);

}  // namespace tempolink::juceapp::network::iceconfig
