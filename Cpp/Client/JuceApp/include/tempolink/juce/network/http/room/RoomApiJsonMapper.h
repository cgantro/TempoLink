#pragma once

#include <vector>

#include <juce_core/juce_core.h>

#include "tempolink/juce/model/RoomSummary.h"

namespace tempolink::juceapp::network::roomapi {

bool ParseCreatedRoom(const juce::String& json_text, RoomSummary& room);
std::vector<RoomSummary> ParseRoomList(const juce::String& json_text);

}  // namespace tempolink::juceapp::network::roomapi

