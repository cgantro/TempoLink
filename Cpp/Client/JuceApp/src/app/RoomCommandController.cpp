#include "tempolink/juce/app/RoomCommandController.h"

#include <utility>

#include <juce_gui_basics/juce_gui_basics.h>

#include "tempolink/juce/constants/ClientText.h"
#include "tempolink/juce/logging/AppLogger.h"

RoomCommandController::RoomCommandController(
    RoomApiClient& room_api, RoomCatalog& room_catalog, MyRoomsView& my_rooms_view,
    std::shared_ptr<std::atomic_bool> alive_flag, std::string& current_user_id_ref)
    : room_api_(room_api),
      room_catalog_(room_catalog),
      my_rooms_view_(my_rooms_view),
      alive_flag_(std::move(alive_flag)),
      current_user_id_ref_(current_user_id_ref) {}

void RoomCommandController::setCallbacks(
    std::function<void()> refresh_room_views,
    std::function<void()> fetch_rooms_from_api,
    std::function<void(const juce::String&)> set_lobby_status_text,
    std::function<void(const std::string&)> open_room_entry) {
  refresh_room_views_ = std::move(refresh_room_views);
  fetch_rooms_from_api_ = std::move(fetch_rooms_from_api);
  set_lobby_status_text_ = std::move(set_lobby_status_text);
  open_room_entry_ = std::move(open_room_entry);
}

void RoomCommandController::createRoom(const std::string& host_user_id,
                                       int max_participants) {
  const std::string host =
      host_user_id.empty() ? current_user_id_ref_ : host_user_id;
  const int clamped_capacity =
      juce::jlimit(2, 6, max_participants <= 0 ? 6 : max_participants);
  if (set_lobby_status_text_) {
    set_lobby_status_text_(tempolink::juceapp::text::kCreatingRoom);
  }

  auto alive = alive_flag_;
  room_api_.createRoomAsync(
      host, clamped_capacity,
      [this, alive](bool ok, RoomSummary room, juce::String error_text) {
        if (!alive->load()) {
          return;
        }
        if (!ok) {
          if (set_lobby_status_text_) {
            set_lobby_status_text_(
                juce::String(tempolink::juceapp::text::kCreateRoomFailedPrefix) +
                error_text);
          }
          return;
        }
        if (room.room_code.empty()) {
          tempolink::juceapp::logging::Error(
              "Create room response missing roomCode");
          if (set_lobby_status_text_) {
            set_lobby_status_text_(
                juce::String(tempolink::juceapp::text::kCreateRoomFailedPrefix) +
                tempolink::juceapp::text::kCreateRoomMissingCode);
          }
          return;
        }

        const std::string created_room_code = room.room_code;
        room_catalog_.upsertRoom(std::move(room));
        if (refresh_room_views_) {
          refresh_room_views_();
        }
        if (fetch_rooms_from_api_) {
          fetch_rooms_from_api_();
        }
        my_rooms_view_.setStatusText("Created room " +
                                     juce::String(created_room_code));
        if (set_lobby_status_text_) {
          set_lobby_status_text_(
              juce::String(tempolink::juceapp::text::kCreatedRoomPrefix) +
              juce::String(created_room_code) +
              tempolink::juceapp::text::kCreatedRoomSuffix);
        }
        if (open_room_entry_) {
          open_room_entry_(created_room_code);
        }
      });
}

void RoomCommandController::editRoom(const std::string& room_code) {
  const RoomSummary* room = room_catalog_.findByCode(room_code);
  if (room == nullptr) {
    my_rooms_view_.setStatusText("Cannot edit: room not found.");
    return;
  }

  auto* dialog = new juce::AlertWindow("Edit Room", "Update room metadata",
                                       juce::AlertWindow::NoIcon, &my_rooms_view_);
  dialog->addTextEditor("name", room->title, "Room Name");
  dialog->addTextEditor("description", room->description, "Description");
  juce::String tags_text;
  for (std::size_t i = 0; i < room->tags.size(); ++i) {
    if (i > 0) {
      tags_text << ", ";
    }
    tags_text << room->tags[i];
  }
  dialog->addTextEditor("tags", tags_text, "Tags (comma separated)");
  juce::StringArray capacity_options;
  for (int i = 2; i <= 6; ++i) {
    capacity_options.add(juce::String(i));
  }
  dialog->addComboBox("capacity", capacity_options, "Max Participants");
  if (auto* combo = dialog->getComboBoxComponent("capacity"); combo != nullptr) {
    combo->setSelectedId(juce::jlimit(1, 5, room->capacity - 1),
                         juce::dontSendNotification);
  }
  juce::StringArray visibility_options;
  visibility_options.add("Public");
  visibility_options.add("Private");
  dialog->addComboBox("visibility", visibility_options, "Visibility");
  if (auto* combo = dialog->getComboBoxComponent("visibility"); combo != nullptr) {
    combo->setSelectedId(room->is_public ? 1 : 2, juce::dontSendNotification);
  }
  juce::StringArray password_options;
  password_options.add("No password");
  password_options.add("Use password");
  dialog->addComboBox("passwordMode", password_options, "Password");
  if (auto* combo = dialog->getComboBoxComponent("passwordMode"); combo != nullptr) {
    combo->setSelectedId(room->has_password ? 2 : 1, juce::dontSendNotification);
  }
  dialog->addTextEditor("password", "", "Password (optional)");
  dialog->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
  dialog->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
  dialog->centreAroundComponent(&my_rooms_view_, 460, 320);
  dialog->setVisible(true);

  dialog->enterModalState(
      true,
      juce::ModalCallbackFunction::create(
          [this, room_code, dialog](int result) {
            std::unique_ptr<juce::AlertWindow> cleanup(dialog);
            if (result != 1) {
              return;
            }

            RoomUpdatePayload payload;
            payload.name = dialog->getTextEditorContents("name").trim();
            payload.description =
                dialog->getTextEditorContents("description").trim();
            const juce::String tags_raw = dialog->getTextEditorContents("tags");
            for (const auto& token :
                 juce::StringArray::fromTokens(tags_raw, ",", "")) {
              const auto trimmed = token.trim();
              if (trimmed.isNotEmpty()) {
                payload.tags.push_back(trimmed);
              }
            }
            if (auto* combo = dialog->getComboBoxComponent("capacity");
                combo != nullptr) {
              payload.max_participants =
                  juce::jlimit(2, 6, combo->getSelectedId() + 1);
            }
            if (auto* combo = dialog->getComboBoxComponent("visibility");
                combo != nullptr) {
              payload.is_public = combo->getSelectedId() != 2;
            }
            if (auto* combo = dialog->getComboBoxComponent("passwordMode");
                combo != nullptr) {
              payload.has_password = combo->getSelectedId() == 2;
            }
            payload.password = dialog->getTextEditorContents("password").trim();

            my_rooms_view_.setStatusText("Updating room " + juce::String(room_code) +
                                         " ...");
            auto alive = alive_flag_;
            room_api_.updateRoomAsync(
                room_code, current_user_id_ref_, payload,
                [this, alive, room_code](bool ok, juce::String error_text) {
                  if (!alive->load()) {
                    return;
                  }
                  if (!ok) {
                    my_rooms_view_.setStatusText("Edit failed: " + error_text);
                    return;
                  }
                  my_rooms_view_.setStatusText("Updated room " +
                                               juce::String(room_code));
                  if (fetch_rooms_from_api_) {
                    fetch_rooms_from_api_();
                  }
                });
          }),
      false);
}

void RoomCommandController::deleteRoom(const std::string& room_code) {
  auto options = juce::MessageBoxOptions()
                     .withIconType(juce::MessageBoxIconType::WarningIcon)
                     .withTitle("Delete Room")
                     .withMessage("Delete room " + juce::String(room_code) + "?")
                     .withButton("Delete")
                     .withButton("Cancel")
                     .withAssociatedComponent(&my_rooms_view_);
  juce::AlertWindow::showAsync(options, [this, room_code](int result) {
    if (result != 1) {
      return;
    }
    my_rooms_view_.setStatusText("Deleting room " + juce::String(room_code) +
                                 " ...");
    auto alive = alive_flag_;
    room_api_.deleteRoomAsync(
        room_code, current_user_id_ref_,
        [this, alive, room_code](bool ok, juce::String error_text) {
          if (!alive->load()) {
            return;
          }
          if (!ok) {
            my_rooms_view_.setStatusText("Delete failed: " + error_text);
            return;
          }
          my_rooms_view_.setStatusText("Deleted room " + juce::String(room_code));
          if (fetch_rooms_from_api_) {
            fetch_rooms_from_api_();
          }
        });
  });
}

void RoomCommandController::shareRoom(const std::string& room_code) {
  const juce::String share_url =
      juce::String("tempolink://room/") + juce::String(room_code);
  juce::SystemClipboard::copyTextToClipboard(share_url);
  my_rooms_view_.setStatusText("Copied share link: " + share_url);
}
