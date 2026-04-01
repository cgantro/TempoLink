#pragma once

#include <functional>
#include <string>
#include <vector>

#include <juce_core/juce_core.h>

#include "tempolink/juce/model/RoomSummary.h"
#include "tempolink/juce/network/http/room/RoomApiTypes.h"

class RoomApiClient {
 public:
  using RoomsCallback = std::function<void(bool, std::vector<RoomSummary>, juce::String)>;
  using SimpleCallback = std::function<void(bool, juce::String)>;
  using CreateCallback = std::function<void(bool, RoomSummary, juce::String)>;

  explicit RoomApiClient(std::string base_url);

  void fetchRoomsAsync(RoomsCallback callback) const;
  void fetchRoomsAsync(const RoomListFilter& filter, RoomsCallback callback) const;
  void createRoomAsync(const std::string& host_user_id, int max_participants,
                       CreateCallback callback) const;
  void joinRoomAsync(const std::string& room_code, const std::string& user_id,
                     SimpleCallback callback) const;
  void leaveRoomAsync(const std::string& room_code, const std::string& user_id,
                      SimpleCallback callback) const;
  void updateRoomAsync(const std::string& room_code, const std::string& host_user_id,
                       const RoomUpdatePayload& payload, SimpleCallback callback) const;
  void deleteRoomAsync(const std::string& room_code, const std::string& host_user_id,
                       SimpleCallback callback) const;

 private:
  static bool postUserAction(const std::string& base_url,
                             const std::string& endpoint, const std::string& user_id,
                             juce::String& error_text);
  static bool payloadAction(const std::string& base_url, const std::string& method,
                            const std::string& endpoint, const juce::String& body,
                            juce::String& error_text);

  std::string base_url_;
};
