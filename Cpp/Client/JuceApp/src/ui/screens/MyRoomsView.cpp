#include "tempolink/juce/ui/screens/MyRoomsView.h"

#include <utility>

#include "tempolink/juce/style/UiStyle.h"
#include "tempolink/juce/ui/components/RoomCardComponent.h"


namespace {
constexpr int kCreateDialogWidth = 480;
constexpr int kCreateDialogHeight = 420;

RoomCreatePayload BuildCreatePayloadFromDialog(juce::AlertWindow& dialog) {
  RoomCreatePayload payload;
  payload.name = dialog.getTextEditorContents("name").trim();
  payload.description = dialog.getTextEditorContents("description").trim();

  const juce::String tags_raw = dialog.getTextEditorContents("tags");
  for (const auto& token : juce::StringArray::fromTokens(tags_raw, ",", "")) {
    const auto trimmed = token.trim();
    if (trimmed.isNotEmpty()) {
      payload.tags.push_back(trimmed);
    }
  }

  if (auto* combo = dialog.getComboBoxComponent("capacity"); combo != nullptr) {
    payload.max_participants = juce::jlimit(2, 6, combo->getSelectedId() + 1);
  }
  if (auto* combo = dialog.getComboBoxComponent("visibility"); combo != nullptr) {
    payload.is_public = combo->getSelectedId() != 2;
  }
  if (auto* combo = dialog.getComboBoxComponent("passwordMode"); combo != nullptr) {
    payload.has_password = combo->getSelectedId() == 2;
  }
  payload.password = dialog.getTextEditorContents("password").trim();
  if (auto* combo = dialog.getComboBoxComponent("timeLimit"); combo != nullptr) {
    const int selected_id = combo->getSelectedId();
    payload.room_time_limit_minutes = selected_id <= 1 ? 0 : (selected_id - 1) * 30;
  }

  return payload;
}
}  // namespace

MyRoomsView::MyRoomsView() {
  
  title_label_.setText("MY ROOMS", juce::dontSendNotification);
  title_label_.setFont(juce::FontOptions(24.0F).withStyle("Bold"));
  addAndMakeVisible(title_label_);

  status_label_.setText("Create and manage your rooms.", juce::dontSendNotification);
  addAndMakeVisible(status_label_);

  back_button_.onClick = [this] {
    if (on_back_) {
      on_back_();
    }
  };
  addAndMakeVisible(back_button_);

  refresh_button_.onClick = [this] {
    if (on_refresh_) {
      on_refresh_();
    }
  };
  addAndMakeVisible(refresh_button_);

  create_button_.onClick = [this] {
    if (!on_create_room_) {
      return;
    }

    auto* dialog = new juce::AlertWindow("Create Room", "Create your room",
                                         juce::AlertWindow::NoIcon, this);
    dialog->addTextEditor("name", "", "Room Name");
    dialog->addTextEditor("description", "", "Comment / Description");
    dialog->addTextEditor("tags", "", "Tags (comma separated)");
    juce::StringArray capacity_options;
    for (int i = 2; i <= 6; ++i) {
      capacity_options.add(juce::String(i));
    }
    dialog->addComboBox("capacity", capacity_options, "Max Participants");
    if (auto* combo = dialog->getComboBoxComponent("capacity"); combo != nullptr) {
      combo->setSelectedId(5, juce::dontSendNotification);
    }
    juce::StringArray visibility_options;
    visibility_options.add("Public");
    visibility_options.add("Private");
    dialog->addComboBox("visibility", visibility_options, "Visibility");
    if (auto* combo = dialog->getComboBoxComponent("visibility"); combo != nullptr) {
      combo->setSelectedId(1, juce::dontSendNotification);
    }

    juce::StringArray password_options;
    password_options.add("No password");
    password_options.add("Use password");
    dialog->addComboBox("passwordMode", password_options, "Password");
    if (auto* combo = dialog->getComboBoxComponent("passwordMode"); combo != nullptr) {
      combo->setSelectedId(1, juce::dontSendNotification);
    }
    dialog->addTextEditor("password", "", "Password (optional)");

    juce::StringArray time_limit_options;
    time_limit_options.add("No limit");
    time_limit_options.add("30 min");
    time_limit_options.add("60 min");
    time_limit_options.add("90 min");
    time_limit_options.add("120 min");
    dialog->addComboBox("timeLimit", time_limit_options, "Room Time Limit");
    if (auto* combo = dialog->getComboBoxComponent("timeLimit"); combo != nullptr) {
      combo->setSelectedId(1, juce::dontSendNotification);
    }
    dialog->addButton("Create", 1, juce::KeyPress(juce::KeyPress::returnKey));
    dialog->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
    dialog->centreAroundComponent(this, kCreateDialogWidth, kCreateDialogHeight);
    dialog->setVisible(true);
    dialog->enterModalState(
        true,
        juce::ModalCallbackFunction::create([safe_this = juce::Component::SafePointer<MyRoomsView>(this),
                                             dialog](int result) {
          std::unique_ptr<juce::AlertWindow> cleanup(dialog);
          if (safe_this == nullptr || result != 1 || !safe_this->on_create_room_) {
            return;
          }
          const RoomCreatePayload payload = BuildCreatePayloadFromDialog(*dialog);
          safe_this->on_create_room_(payload);
        }),
        false);
  };
  addAndMakeVisible(create_button_);

  cards_viewport_.setViewedComponent(&cards_container_, false);
  cards_viewport_.setScrollBarsShown(true, false);
  addAndMakeVisible(cards_viewport_);

  updateTheme();
}
void MyRoomsView::updateTheme() {
  title_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());
  status_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextSecondary());
  
  auto update_btn = [](juce::TextButton& btn) {
    btn.setColour(juce::TextButton::buttonColourId, tempolink::juceapp::style::PanelBackground());
    btn.setColour(juce::TextButton::textColourOffId, tempolink::juceapp::style::TextPrimary());
  };
  update_btn(back_button_);
  update_btn(refresh_button_);
  update_btn(create_button_);

  repaint();
}

void MyRoomsView::setRooms(const std::vector<RoomSummary>& rooms) {
  rooms_ = rooms;
  rebuildRoomCards();
  resized();
}

void MyRoomsView::setStatusText(const std::string& status_text) {
  status_label_.setText(juce::String(status_text), juce::dontSendNotification);
}

void MyRoomsView::setPreviewHandler(std::function<void(std::string)> on_preview_room) {
  on_preview_room_ = std::move(on_preview_room);
  for (auto& card : room_cards_) {
    card->setPreviewHandler(on_preview_room_);
  }
}

void MyRoomsView::setEnterHandler(std::function<void(std::string)> on_enter_room) {
  on_enter_room_ = std::move(on_enter_room);
  for (auto& card : room_cards_) {
    card->setEnterHandler(on_enter_room_);
  }
}

void MyRoomsView::setEditHandler(std::function<void(std::string)> on_edit_room) {
  on_edit_room_ = std::move(on_edit_room);
  for (auto& card : room_cards_) {
    card->setEditHandler(on_edit_room_);
  }
}

void MyRoomsView::setDeleteHandler(std::function<void(std::string)> on_delete_room) {
  on_delete_room_ = std::move(on_delete_room);
  for (auto& card : room_cards_) {
    card->setDeleteHandler(on_delete_room_);
  }
}

void MyRoomsView::setShareHandler(std::function<void(std::string)> on_share_room) {
  on_share_room_ = std::move(on_share_room);
  for (auto& card : room_cards_) {
    card->setShareHandler(on_share_room_);
  }
}

void MyRoomsView::setCreateHandler(
    std::function<void(const RoomCreatePayload&)> on_create_room) {
  on_create_room_ = std::move(on_create_room);
}

void MyRoomsView::setRefreshHandler(std::function<void()> on_refresh) {
  on_refresh_ = std::move(on_refresh);
}

void MyRoomsView::setBackHandler(std::function<void()> on_back) {
  on_back_ = std::move(on_back);
}

void MyRoomsView::resized() {
  auto area = getLocalBounds().reduced(tempolink::juceapp::style::kLobbyPadding);

  auto header = area.removeFromTop(42);
  title_label_.setBounds(header.removeFromLeft(220));
  back_button_.setBounds(header.removeFromRight(130));

  area.removeFromTop(6);
  auto controls = area.removeFromTop(32);
  create_button_.setBounds(controls.removeFromLeft(140));
  controls.removeFromLeft(8);
  refresh_button_.setBounds(controls.removeFromLeft(100));
  status_label_.setBounds(controls);

  area.removeFromTop(8);
  cards_viewport_.setBounds(area);
  layoutRoomCards();
}

void MyRoomsView::paint(juce::Graphics& g) {
  g.fillAll(tempolink::juceapp::style::LobbyBackground());
}

void MyRoomsView::rebuildRoomCards() {
  room_cards_.clear();
  cards_container_.removeAllChildren();

  for (const auto& room : rooms_) {
    auto card = std::make_unique<RoomCardComponent>();
    card->setMode(RoomCardComponent::CardMode::MyRooms);
    card->setRoom(room);
    card->setPreviewHandler(on_preview_room_);
    card->setEnterHandler(on_enter_room_);
    card->setEditHandler(on_edit_room_);
    card->setDeleteHandler(on_delete_room_);
    card->setShareHandler(on_share_room_);
    cards_container_.addAndMakeVisible(*card);
    room_cards_.push_back(std::move(card));
  }
}

void MyRoomsView::layoutRoomCards() {
  if (cards_viewport_.getWidth() <= 0) {
    return;
  }

  const int viewport_width = juce::jmax(1, cards_viewport_.getMaximumVisibleWidth());
  const int padding = tempolink::juceapp::style::kLobbyGridPadding;
  const int min_card_width = 240;
  int column_count = (viewport_width - padding) / (min_card_width + padding);
  column_count = juce::jlimit(1, 4, column_count);
  const int column_width = (viewport_width - padding * (column_count + 1)) / column_count;
  const int card_height = tempolink::juceapp::style::kLobbyCardHeight;

  int x = padding;
  int y = padding;
  int col = 0;
  for (auto& card : room_cards_) {
    card->setBounds(x, y, column_width, card_height);
    ++col;
    if (col >= column_count) {
      col = 0;
      x = padding;
      y += card_height + padding;
    } else {
      x += column_width + padding;
    }
  }

  const int final_height = room_cards_.empty() ? (padding * 2) : (y + card_height + padding);
  cards_container_.setSize(viewport_width, final_height);
}
