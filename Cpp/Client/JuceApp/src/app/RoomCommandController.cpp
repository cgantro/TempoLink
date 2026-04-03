#include "tempolink/juce/app/RoomCommandController.h"

#include <utility>

#include "tempolink/juce/constants/ClientText.h"
#include "tempolink/juce/logging/AppLogger.h"

RoomCommandController::RoomCommandController(Dependencies dependencies,
                                             Callbacks callbacks)
    : room_api_(dependencies.room_api),
      room_catalog_(dependencies.room_catalog),
      alive_flag_(std::move(dependencies.alive_flag)),
      current_user_id_ref_(dependencies.current_user_id_ref),
      callbacks_(std::move(callbacks)) {}

void RoomCommandController::createRoom(const std::string& host_user_id,
                                       int max_participants) {
  RoomCreatePayload payload;
  payload.max_participants = max_participants;
  createRoom(host_user_id, payload);
}

void RoomCommandController::createRoom(const std::string& host_user_id,
                                       const RoomCreatePayload& payload) {
  if (current_user_id_ref_.empty() && host_user_id.empty()) {
    const int guest_suffix = 100000 + juce::Random::getSystemRandom().nextInt(900000);
    current_user_id_ref_ = "local-guest-" + std::to_string(guest_suffix);
  }
  const std::string host = host_user_id.empty() ? current_user_id_ref_ : host_user_id;
  RoomCreatePayload normalized_payload = payload;
  normalized_payload.max_participants =
      juce::jlimit(2, 6,
                   normalized_payload.max_participants <= 0
                       ? 6
                       : normalized_payload.max_participants);
  if (callbacks_.set_lobby_status_text) {
    callbacks_.set_lobby_status_text(tempolink::juceapp::text::kCreatingRoom);
  }

  auto alive = alive_flag_;
  room_api_.createRoomAsync(
      host, normalized_payload,
      [this, alive, host, normalized_payload](bool ok, RoomSummary room,
                                              juce::String error_text) {
        if (!alive->load()) {
          return;
        }
        if (!ok) {
          if (callbacks_.set_lobby_status_text) {
            callbacks_.set_lobby_status_text(
                juce::String(tempolink::juceapp::text::kCreateRoomFailedPrefix) +
                error_text);
          }
          return;
        }
        if (room.room_code.empty()) {
          tempolink::juceapp::logging::Error(
              "Create room response missing roomCode");
          if (callbacks_.set_lobby_status_text) {
            callbacks_.set_lobby_status_text(
                juce::String(tempolink::juceapp::text::kCreateRoomFailedPrefix) +
                tempolink::juceapp::text::kCreateRoomMissingCode);
          }
          return;
        }

        const std::string created_room_code = room.room_code;
        room_catalog_.upsertRoom(std::move(room));
        if (callbacks_.refresh_room_views) {
          callbacks_.refresh_room_views();
        }
        if (callbacks_.fetch_rooms_from_api) {
          callbacks_.fetch_rooms_from_api();
        }
        if (callbacks_.set_my_rooms_status_text) {
          callbacks_.set_my_rooms_status_text("Created room " + created_room_code);
        }
        if (callbacks_.set_lobby_status_text) {
          callbacks_.set_lobby_status_text(
              juce::String(tempolink::juceapp::text::kCreatedRoomPrefix) +
              juce::String(created_room_code) +
              tempolink::juceapp::text::kCreatedRoomSuffix);
        }

        RoomUpdatePayload update_payload;
        update_payload.name = normalized_payload.name;
        update_payload.description = normalized_payload.description;
        update_payload.tags = normalized_payload.tags;
        update_payload.is_public = normalized_payload.is_public;
        update_payload.has_password = normalized_payload.has_password;
        update_payload.password = normalized_payload.password;
        update_payload.max_participants = normalized_payload.max_participants;

        const bool has_metadata =
            update_payload.name.isNotEmpty() || update_payload.description.isNotEmpty() ||
            !update_payload.tags.empty() || !update_payload.is_public ||
            update_payload.has_password;

        if (!has_metadata) {
          if (callbacks_.open_room_entry) {
            callbacks_.open_room_entry(created_room_code);
          }
          return;
        }

        room_api_.updateRoomAsync(
            created_room_code, host, update_payload,
            [this, alive, created_room_code](bool update_ok, juce::String update_error) {
              if (!alive->load()) {
                return;
              }
              if (!update_ok && callbacks_.set_lobby_status_text) {
                callbacks_.set_lobby_status_text(
                    "Room created, but metadata update failed: " + update_error);
              }
              if (callbacks_.fetch_rooms_from_api) {
                callbacks_.fetch_rooms_from_api();
              }
              if (callbacks_.open_room_entry) {
                callbacks_.open_room_entry(created_room_code);
              }
            });
      });
}

void RoomCommandController::editRoom(const std::string& room_code) {
  const RoomSummary* room = room_catalog_.findByCode(room_code);
  if (room == nullptr) {
    if (callbacks_.set_my_rooms_status_text) {
      callbacks_.set_my_rooms_status_text("Cannot edit: room not found.");
    }
    return;
  }

  if (!callbacks_.request_room_edit_payload) {
    return;
  }

  callbacks_.request_room_edit_payload(
      *room, [this, room_code](std::optional<RoomUpdatePayload> payload) {
        if (!payload.has_value()) {
          return;
        }

        if (callbacks_.set_my_rooms_status_text) {
          callbacks_.set_my_rooms_status_text("Updating room " + room_code + " ...");
        }
        auto alive = alive_flag_;
        room_api_.updateRoomAsync(
            room_code, current_user_id_ref_, *payload,
            [this, alive, room_code](bool ok, juce::String error_text) {
              if (!alive->load()) {
                return;
              }
              if (!ok) {
                if (callbacks_.set_my_rooms_status_text) {
                  callbacks_.set_my_rooms_status_text(
                      "Edit failed: " + error_text.toStdString());
                }
                return;
              }
              if (callbacks_.set_my_rooms_status_text) {
                callbacks_.set_my_rooms_status_text("Updated room " + room_code);
              }
              if (callbacks_.fetch_rooms_from_api) {
                callbacks_.fetch_rooms_from_api();
              }
            });
      });
}

void RoomCommandController::deleteRoom(const std::string& room_code) {
  if (!callbacks_.request_delete_confirmation) {
    return;
  }

  callbacks_.request_delete_confirmation(room_code, [this, room_code](bool confirmed) {
    if (!confirmed) {
      return;
    }
    if (callbacks_.set_my_rooms_status_text) {
      callbacks_.set_my_rooms_status_text("Deleting room " + room_code + " ...");
    }
    auto alive = alive_flag_;
    room_api_.deleteRoomAsync(
        room_code, current_user_id_ref_,
        [this, alive, room_code](bool ok, juce::String error_text) {
          if (!alive->load()) {
            return;
          }
          if (!ok) {
            if (callbacks_.set_my_rooms_status_text) {
              callbacks_.set_my_rooms_status_text(
                  "Delete failed: " + error_text.toStdString());
            }
            return;
          }
          if (callbacks_.set_my_rooms_status_text) {
            callbacks_.set_my_rooms_status_text("Deleted room " + room_code);
          }
          if (callbacks_.fetch_rooms_from_api) {
            callbacks_.fetch_rooms_from_api();
          }
        });
  });
}

void RoomCommandController::shareRoom(const std::string& room_code) {
  const juce::String share_url =
      juce::String("tempolink://room/") + juce::String(room_code);
  if (callbacks_.copy_to_clipboard) {
    callbacks_.copy_to_clipboard(share_url);
  } else if (callbacks_.set_my_rooms_status_text) {
    callbacks_.set_my_rooms_status_text("Clipboard unavailable.");
    return;
  }
  if (callbacks_.set_my_rooms_status_text) {
    callbacks_.set_my_rooms_status_text("Copied share link: " + share_url.toStdString());
  }
}
