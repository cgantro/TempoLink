#pragma once

#include <string>

#include <juce_core/juce_core.h>

#include "tempolink/juce/network/http/room/RoomApiTypes.h"

namespace tempolink::juceapp::network::roomapi {

std::string BuildRoomListPath(const RoomListFilter& filter);
juce::String BuildUserActionBody(const std::string& user_id);
juce::String BuildCreateRoomBody(const std::string& host_user_id,
                                 int max_participants);
juce::String BuildCreateRoomBody(const std::string& host_user_id,
                                 const RoomCreatePayload& payload);
juce::String BuildUpdateRoomBody(const std::string& host_user_id,
                                 const RoomUpdatePayload& payload);
juce::String BuildDeleteRoomBody(const std::string& host_user_id);

}  // namespace tempolink::juceapp::network::roomapi
