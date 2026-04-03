#include "tempolink/juce/network/http/room/RoomApiClient.h"

#include <utility>

#include <juce_events/juce_events.h>

#include "tempolink/juce/constants/ClientText.h"
#include "tempolink/juce/network/http/common/HttpTransport.h"
#include "tempolink/juce/network/http/room/RoomApiJsonMapper.h"
#include "tempolink/juce/network/http/room/RoomApiPayloadBuilder.h"

namespace {

namespace http = tempolink::juceapp::network::http;
namespace roomapi = tempolink::juceapp::network::roomapi;

}  // namespace

RoomApiClient::RoomApiClient(std::string base_url)
    : base_url_(http::NormalizeBaseUrl(std::move(base_url))),
      runner_(std::make_shared<tempolink::juceapp::async::AsyncTaskRunner>(
          std::make_shared<std::atomic_bool>(true))) {}

RoomApiClient::RoomApiClient(RoomApiClient&& other) noexcept {
  std::lock_guard<std::mutex> lock(other.runner_mutex_);
  base_url_ = std::move(other.base_url_);
  runner_ = std::move(other.runner_);
}

RoomApiClient& RoomApiClient::operator=(RoomApiClient&& other) noexcept {
  if (this == &other) {
    return *this;
  }
  std::scoped_lock lock(runner_mutex_, other.runner_mutex_);
  base_url_ = std::move(other.base_url_);
  runner_ = std::move(other.runner_);
  return *this;
}

void RoomApiClient::SetAsyncRunner(
    std::shared_ptr<tempolink::juceapp::async::AsyncTaskRunner> runner) {
  std::lock_guard<std::mutex> lock(runner_mutex_);
  runner_ = std::move(runner);
}

void RoomApiClient::fetchRoomsAsync(RoomsCallback callback) const {
  fetchRoomsAsync(RoomListFilter{}, std::move(callback));
}

void RoomApiClient::fetchRoomsAsync(const RoomListFilter& filter,
                                    RoomsCallback callback) const {
  std::shared_ptr<tempolink::juceapp::async::AsyncTaskRunner> runner;
  {
    std::lock_guard<std::mutex> lock(runner_mutex_);
    runner = runner_;
  }
  if (!runner) {
    juce::MessageManager::callAsync([callback = std::move(callback)]() mutable {
      callback(false, {}, "Room API runner is not configured.");
    });
    return;
  }

  runner->RunAsync(
      [base_url = base_url_, filter, callback = std::move(callback)]() mutable {
        juce::String error_text;
        std::vector<RoomSummary> rooms;

        http::Request request;
        request.method = "GET";
        request.path = roomapi::BuildRoomListPath(filter);
        request.timeout_ms = 3500;
        const http::Response response = http::Perform(base_url, request);
        if (!response.transport_ok) {
          error_text = tempolink::juceapp::text::kRoomApiConnectFailed;
        } else if (response.isHttpSuccess()) {
          rooms = roomapi::ParseRoomList(response.body);
        } else {
          error_text = juce::String(tempolink::juceapp::text::kRoomApiHttpErrorPrefix) +
                       juce::String(response.status_code);
        }

        juce::MessageManager::callAsync(
            [callback = std::move(callback), rooms = std::move(rooms),
             error_text]() mutable {
              callback(error_text.isEmpty(), std::move(rooms), error_text);
            });
      });
}

void RoomApiClient::createRoomAsync(const std::string& host_user_id,
                                    int max_participants,
                                    CreateCallback callback) const {
  RoomCreatePayload payload;
  payload.max_participants = max_participants;
  createRoomAsync(host_user_id, payload, std::move(callback));
}

void RoomApiClient::createRoomAsync(const std::string& host_user_id,
                                    const RoomCreatePayload& payload,
                                    CreateCallback callback) const {
  std::shared_ptr<tempolink::juceapp::async::AsyncTaskRunner> runner;
  {
    std::lock_guard<std::mutex> lock(runner_mutex_);
    runner = runner_;
  }
  if (!runner) {
    juce::MessageManager::callAsync([callback = std::move(callback)]() mutable {
      callback(false, {}, "Room API runner is not configured.");
    });
    return;
  }

  runner->RunAsync(
      [base_url = base_url_, host_user_id, payload,
       callback = std::move(callback)]() mutable {
        juce::String error_text;
        RoomSummary room;

        http::Request request;
        request.method = "POST";
        request.path = "/api/rooms";
        request.body = roomapi::BuildCreateRoomBody(host_user_id, payload);
        request.timeout_ms = 3500;

        const http::Response response = http::Perform(base_url, request);
        if (!response.transport_ok) {
          error_text = tempolink::juceapp::text::kCreateRoomApiCallFailed;
        } else if (!response.isHttpSuccess()) {
          error_text = juce::String(tempolink::juceapp::text::kCreateRoomApiHttpErrorPrefix) +
                       juce::String(response.status_code);
        } else {
          const bool parsed_ok = roomapi::ParseCreatedRoom(response.body, room);
          if (!parsed_ok || room.room_code.empty()) {
            error_text = tempolink::juceapp::text::kCreateRoomInvalidResponse;
          }
        }

        juce::MessageManager::callAsync(
            [callback = std::move(callback), ok = error_text.isEmpty(),
             room = std::move(room), error_text]() mutable {
              callback(ok, std::move(room), error_text);
            });
      });
}

void RoomApiClient::joinRoomAsync(const std::string& room_code,
                                  const std::string& user_id,
                                  SimpleCallback callback) const {
  std::shared_ptr<tempolink::juceapp::async::AsyncTaskRunner> runner;
  {
    std::lock_guard<std::mutex> lock(runner_mutex_);
    runner = runner_;
  }
  if (!runner) {
    juce::MessageManager::callAsync([callback = std::move(callback)]() mutable {
      callback(false, "Room API runner is not configured.");
    });
    return;
  }

  runner->RunAsync(
      [base_url = base_url_, room_code, user_id,
       callback = std::move(callback)]() mutable {
        juce::String error_text;
        const bool ok = postUserAction(
            base_url, "/api/rooms/" + http::EscapePathSegment(room_code) + "/join",
            user_id, error_text);

        juce::MessageManager::callAsync(
            [callback = std::move(callback), ok, error_text]() mutable {
              callback(ok, error_text);
            });
      });
}

void RoomApiClient::leaveRoomAsync(const std::string& room_code,
                                   const std::string& user_id,
                                   SimpleCallback callback) const {
  std::shared_ptr<tempolink::juceapp::async::AsyncTaskRunner> runner;
  {
    std::lock_guard<std::mutex> lock(runner_mutex_);
    runner = runner_;
  }
  if (!runner) {
    juce::MessageManager::callAsync([callback = std::move(callback)]() mutable {
      callback(false, "Room API runner is not configured.");
    });
    return;
  }

  runner->RunAsync(
      [base_url = base_url_, room_code, user_id,
       callback = std::move(callback)]() mutable {
        juce::String error_text;
        const bool ok = postUserAction(
            base_url, "/api/rooms/" + http::EscapePathSegment(room_code) + "/leave",
            user_id, error_text);

        juce::MessageManager::callAsync(
            [callback = std::move(callback), ok, error_text]() mutable {
              callback(ok, error_text);
            });
      });
}

void RoomApiClient::updateRoomAsync(const std::string& room_code,
                                    const std::string& host_user_id,
                                    const RoomUpdatePayload& payload,
                                    SimpleCallback callback) const {
  std::shared_ptr<tempolink::juceapp::async::AsyncTaskRunner> runner;
  {
    std::lock_guard<std::mutex> lock(runner_mutex_);
    runner = runner_;
  }
  if (!runner) {
    juce::MessageManager::callAsync([callback = std::move(callback)]() mutable {
      callback(false, "Room API runner is not configured.");
    });
    return;
  }

  runner->RunAsync(
      [base_url = base_url_, room_code, host_user_id, payload,
       callback = std::move(callback)]() mutable {
        juce::String error_text;
        const juce::String body = roomapi::BuildUpdateRoomBody(host_user_id, payload);
        const bool ok = payloadAction(
            base_url, "PUT",
            "/api/rooms/" + http::EscapePathSegment(room_code), body, error_text);

        juce::MessageManager::callAsync(
            [callback = std::move(callback), ok, error_text]() mutable {
              callback(ok, error_text);
            });
      });
}

void RoomApiClient::deleteRoomAsync(const std::string& room_code,
                                    const std::string& host_user_id,
                                    SimpleCallback callback) const {
  std::shared_ptr<tempolink::juceapp::async::AsyncTaskRunner> runner;
  {
    std::lock_guard<std::mutex> lock(runner_mutex_);
    runner = runner_;
  }
  if (!runner) {
    juce::MessageManager::callAsync([callback = std::move(callback)]() mutable {
      callback(false, "Room API runner is not configured.");
    });
    return;
  }

  runner->RunAsync(
      [base_url = base_url_, room_code, host_user_id,
       callback = std::move(callback)]() mutable {
        juce::String error_text;
        const juce::String body = roomapi::BuildDeleteRoomBody(host_user_id);
        const bool ok = payloadAction(
            base_url, "DELETE",
            "/api/rooms/" + http::EscapePathSegment(room_code), body, error_text);

        juce::MessageManager::callAsync(
            [callback = std::move(callback), ok, error_text]() mutable {
              callback(ok, error_text);
            });
      });
}

bool RoomApiClient::postUserAction(const std::string& base_url,
                                   const std::string& endpoint,
                                   const std::string& user_id,
                                   juce::String& error_text) {
  return payloadAction(base_url, "POST", endpoint,
                       roomapi::BuildUserActionBody(user_id), error_text);
}

bool RoomApiClient::payloadAction(const std::string& base_url,
                                  const std::string& method,
                                  const std::string& endpoint,
                                  const juce::String& body,
                                  juce::String& error_text) {
  http::Request request;
  request.method = method;
  request.path = endpoint;
  request.body = body;
  request.timeout_ms = 3500;
  const http::Response response = http::Perform(base_url, request);
  if (!response.transport_ok) {
    error_text = tempolink::juceapp::text::kRoomActionApiCallFailed;
    return false;
  }
  if (!response.isHttpSuccess()) {
    error_text = juce::String(tempolink::juceapp::text::kRoomActionApiHttpErrorPrefix) +
                 juce::String(response.status_code);
    const juce::String body = response.body.substring(0, 180).trim();
    if (body.isNotEmpty()) {
      error_text += ": " + body;
    }
    return false;
  }
  return true;
}
