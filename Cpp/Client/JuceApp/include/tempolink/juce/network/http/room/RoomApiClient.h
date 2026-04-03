#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <juce_core/juce_core.h>

#include "tempolink/juce/async/AsyncTaskRunner.h"
#include "tempolink/juce/model/RoomSummary.h"
#include "tempolink/juce/network/http/room/RoomApiTypes.h"

class RoomApiClient {
 public:
  using RoomsCallback = std::function<void(bool, std::vector<RoomSummary>, juce::String)>;
  using SimpleCallback = std::function<void(bool, juce::String)>;
  using CreateCallback = std::function<void(bool, RoomSummary, juce::String)>;

  explicit RoomApiClient(std::string base_url);
  RoomApiClient(const RoomApiClient&) = delete;
  RoomApiClient& operator=(const RoomApiClient&) = delete;
  RoomApiClient(RoomApiClient&& other) noexcept;
  RoomApiClient& operator=(RoomApiClient&& other) noexcept;

  /// Override the async runner used by API calls.
  void SetAsyncRunner(std::shared_ptr<tempolink::juceapp::async::AsyncTaskRunner> runner);

  void fetchRoomsAsync(RoomsCallback callback) const;
  void fetchRoomsAsync(const RoomListFilter& filter, RoomsCallback callback) const;
  void createRoomAsync(const std::string& host_user_id, int max_participants,
                       CreateCallback callback) const;
  void createRoomAsync(const std::string& host_user_id, const RoomCreatePayload& payload,
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
  mutable std::mutex runner_mutex_;
  mutable std::shared_ptr<tempolink::juceapp::async::AsyncTaskRunner> runner_;
};
