#pragma once

#include <functional>
#include <optional>
#include <string>

#include <juce_core/juce_core.h>

#include "tempolink/juce/network/http/room/RoomApiClient.h"

namespace juce {
class Component;
}

namespace tempolink::juceapp::app {

class RoomCommandUiCoordinator {
 public:
  explicit RoomCommandUiCoordinator(juce::Component& dialog_parent);

  void requestEditRoomPayload(
      const RoomSummary& room,
      std::function<void(std::optional<RoomUpdatePayload>)> on_complete);
  void requestDeleteConfirmation(const std::string& room_code,
                                 std::function<void(bool)> on_complete);
  void copyToClipboard(const juce::String& text);

 private:
  juce::Component& dialog_parent_;
};

}  // namespace tempolink::juceapp::app
