#pragma once

#include <functional>
#include <string>
#include <vector>

#include <juce_core/juce_core.h>

#include "tempolink/juce/ui/UiModels.h"

class RoomApiClient {
 public:
  using RoomsCallback = std::function<void(bool, std::vector<RoomSummary>, juce::String)>;
  using SimpleCallback = std::function<void(bool, juce::String)>;

  explicit RoomApiClient(std::string base_url);

  void fetchRoomsAsync(RoomsCallback callback) const;
  void joinRoomAsync(const std::string& room_code, const std::string& user_id,
                     SimpleCallback callback) const;
  void leaveRoomAsync(const std::string& room_code, const std::string& user_id,
                      SimpleCallback callback) const;

 private:
  static bool postUserAction(const std::string& endpoint, const std::string& user_id,
                             juce::String& error_text);
  static std::vector<RoomSummary> parseRooms(const juce::String& json_text);

  std::string base_url_;
};
