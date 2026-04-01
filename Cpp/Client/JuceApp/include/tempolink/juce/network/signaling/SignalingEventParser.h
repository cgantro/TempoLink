#pragma once

#include <optional>

#include "tempolink/juce/network/signaling/SignalingClient.h"

namespace tempolink::juceapp::network {

std::optional<SignalingClient::Event> ParseSignalingEvent(const juce::String& message_text);

}  // namespace tempolink::juceapp::network

