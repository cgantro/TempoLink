#include "tempolink/juce/app/navigation/coordinators/RoomCommandUiCoordinator.h"

#include <memory>
#include <utility>

#include <juce_gui_basics/juce_gui_basics.h>

namespace tempolink::juceapp::app {

RoomCommandUiCoordinator::RoomCommandUiCoordinator(juce::Component& dialog_parent)
    : dialog_parent_(dialog_parent) {}

void RoomCommandUiCoordinator::requestEditRoomPayload(
    const RoomSummary& room,
    std::function<void(std::optional<RoomUpdatePayload>)> on_complete) {
  auto* dialog = new juce::AlertWindow("Edit Room", "Update room metadata",
                                       juce::AlertWindow::NoIcon, &dialog_parent_);
  dialog->addTextEditor("name", room.title, "Room Name");
  dialog->addTextEditor("description", room.description, "Description");

  juce::String tags_text;
  for (std::size_t i = 0; i < room.tags.size(); ++i) {
    if (i > 0) {
      tags_text << ", ";
    }
    tags_text << room.tags[i];
  }
  dialog->addTextEditor("tags", tags_text, "Tags (comma separated)");

  juce::StringArray capacity_options;
  for (int i = 2; i <= 6; ++i) {
    capacity_options.add(juce::String(i));
  }
  dialog->addComboBox("capacity", capacity_options, "Max Participants");
  if (auto* combo = dialog->getComboBoxComponent("capacity"); combo != nullptr) {
    combo->setSelectedId(juce::jlimit(1, 5, room.capacity - 1),
                         juce::dontSendNotification);
  }

  juce::StringArray visibility_options;
  visibility_options.add("Public");
  visibility_options.add("Private");
  dialog->addComboBox("visibility", visibility_options, "Visibility");
  if (auto* combo = dialog->getComboBoxComponent("visibility"); combo != nullptr) {
    combo->setSelectedId(room.is_public ? 1 : 2, juce::dontSendNotification);
  }

  juce::StringArray password_options;
  password_options.add("No password");
  password_options.add("Use password");
  dialog->addComboBox("passwordMode", password_options, "Password");
  if (auto* combo = dialog->getComboBoxComponent("passwordMode"); combo != nullptr) {
    combo->setSelectedId(room.has_password ? 2 : 1, juce::dontSendNotification);
  }

  dialog->addTextEditor("password", "", "Password (optional)");
  dialog->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
  dialog->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
  dialog->centreAroundComponent(&dialog_parent_, 460, 320);
  dialog->setVisible(true);

  dialog->enterModalState(
      true,
      juce::ModalCallbackFunction::create(
          [dialog, on_complete = std::move(on_complete)](int result) mutable {
            std::unique_ptr<juce::AlertWindow> cleanup(dialog);
            if (result != 1) {
              if (on_complete) {
                on_complete(std::nullopt);
              }
              return;
            }

            RoomUpdatePayload payload;
            payload.name = dialog->getTextEditorContents("name").trim();
            payload.description = dialog->getTextEditorContents("description").trim();
            const juce::String tags_raw = dialog->getTextEditorContents("tags");
            for (const auto& token : juce::StringArray::fromTokens(tags_raw, ",", "")) {
              const auto trimmed = token.trim();
              if (trimmed.isNotEmpty()) {
                payload.tags.push_back(trimmed);
              }
            }
            if (auto* combo = dialog->getComboBoxComponent("capacity"); combo != nullptr) {
              payload.max_participants = juce::jlimit(2, 6, combo->getSelectedId() + 1);
            }
            if (auto* combo = dialog->getComboBoxComponent("visibility"); combo != nullptr) {
              payload.is_public = combo->getSelectedId() != 2;
            }
            if (auto* combo = dialog->getComboBoxComponent("passwordMode"); combo != nullptr) {
              payload.has_password = combo->getSelectedId() == 2;
            }
            payload.password = dialog->getTextEditorContents("password").trim();

            if (on_complete) {
              on_complete(payload);
            }
          }),
      false);
}

void RoomCommandUiCoordinator::requestDeleteConfirmation(
    const std::string& room_code, std::function<void(bool)> on_complete) {
  auto options = juce::MessageBoxOptions()
                     .withIconType(juce::MessageBoxIconType::WarningIcon)
                     .withTitle("Delete Room")
                     .withMessage("Delete room " + juce::String(room_code) + "?")
                     .withButton("Delete")
                     .withButton("Cancel")
                     .withAssociatedComponent(&dialog_parent_);
  juce::AlertWindow::showAsync(
      options, [on_complete = std::move(on_complete)](int result) {
        if (on_complete) {
          on_complete(result == 1);
        }
      });
}

void RoomCommandUiCoordinator::copyToClipboard(const juce::String& text) {
  juce::SystemClipboard::copyTextToClipboard(text);
}

}  // namespace tempolink::juceapp::app
