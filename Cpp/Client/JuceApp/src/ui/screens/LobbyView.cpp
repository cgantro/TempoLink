#include "tempolink/juce/ui/screens/LobbyView.h"

#include <algorithm>
#include <array>
#include <memory>
#include <utility>

#include "tempolink/juce/style/UiStyle.h"

LobbyView::LobbyView() {
  title_label_.setText("Rooms", juce::dontSendNotification);
  title_label_.setFont(juce::FontOptions(24.0F).withStyle("Bold"));
  title_label_.setColour(juce::Label::textColourId,
                         tempolink::juceapp::style::TextPrimary());
  addAndMakeVisible(title_label_);

  tabs_label_.setText("Active Rooms      Standby Rooms", juce::dontSendNotification);
  tabs_label_.setColour(juce::Label::textColourId,
                        tempolink::juceapp::style::TextPrimary());
  addAndMakeVisible(tabs_label_);

  status_label_.setText("Select a room and join session.", juce::dontSendNotification);
  status_label_.setColour(juce::Label::textColourId,
                          tempolink::juceapp::style::TextSecondary());
  addAndMakeVisible(status_label_);

  logo_image_.setInterceptsMouseClicks(false, false);
  logo_image_.setVisible(false);
  addAndMakeVisible(logo_image_);

  logo_text_label_.setText("TempoLink", juce::dontSendNotification);
  logo_text_label_.setFont(juce::FontOptions(20.0F).withStyle("Bold"));
  logo_text_label_.setColour(juce::Label::textColourId,
                             tempolink::juceapp::style::TextPrimary());
  addAndMakeVisible(logo_text_label_);

  nav_title_label_.setText("Navigation", juce::dontSendNotification);
  nav_title_label_.setFont(juce::FontOptions(12.0F).withStyle("Bold"));
  nav_title_label_.setColour(juce::Label::textColourId,
                             tempolink::juceapp::style::TextSecondary());
  addAndMakeVisible(nav_title_label_);

  search_editor_.setTextToShowWhenEmpty("Search rooms",
                                        tempolink::juceapp::style::TextSecondary());
  search_editor_.setColour(juce::TextEditor::textColourId,
                           tempolink::juceapp::style::TextPrimary());
  search_editor_.setColour(juce::TextEditor::backgroundColourId,
                           tempolink::juceapp::style::PanelBackground());
  search_editor_.setColour(juce::TextEditor::outlineColourId,
                           tempolink::juceapp::style::BorderStrong());
  search_editor_.onTextChange = [this] {
    filter_.query = search_editor_.getText().trim().toStdString();
    rebuildRoomCards();
    layoutRoomCards();
    if (on_filter_changed_) {
      on_filter_changed_(filter_);
    }
  };
  addAndMakeVisible(search_editor_);

  tag_filter_editor_.setTextToShowWhenEmpty("Tag filter",
                                            tempolink::juceapp::style::TextSecondary());
  tag_filter_editor_.setColour(juce::TextEditor::textColourId,
                               tempolink::juceapp::style::TextPrimary());
  tag_filter_editor_.setColour(juce::TextEditor::backgroundColourId,
                               tempolink::juceapp::style::PanelBackground());
  tag_filter_editor_.setColour(juce::TextEditor::outlineColourId,
                               tempolink::juceapp::style::BorderStrong());
  tag_filter_editor_.onTextChange = [this] {
    filter_.tag = tag_filter_editor_.getText().trim().toStdString();
    rebuildRoomCards();
    layoutRoomCards();
    if (on_filter_changed_) {
      on_filter_changed_(filter_);
    }
  };
  addAndMakeVisible(tag_filter_editor_);

  visibility_filter_combo_.addItem("Visibility: All", 1);
  visibility_filter_combo_.addItem("Public only", 2);
  visibility_filter_combo_.addItem("Private only", 3);
  visibility_filter_combo_.setSelectedId(1, juce::dontSendNotification);
  visibility_filter_combo_.onChange = [this] {
    switch (visibility_filter_combo_.getSelectedId()) {
      case 2:
        filter_.is_public = true;
        break;
      case 3:
        filter_.is_public = false;
        break;
      default:
        filter_.is_public.reset();
        break;
    }
    rebuildRoomCards();
    layoutRoomCards();
    if (on_filter_changed_) {
      on_filter_changed_(filter_);
    }
  };
  addAndMakeVisible(visibility_filter_combo_);

  password_filter_combo_.addItem("Password: All", 1);
  password_filter_combo_.addItem("With password", 2);
  password_filter_combo_.addItem("No password", 3);
  password_filter_combo_.setSelectedId(1, juce::dontSendNotification);
  password_filter_combo_.onChange = [this] {
    switch (password_filter_combo_.getSelectedId()) {
      case 2:
        filter_.has_password = true;
        break;
      case 3:
        filter_.has_password = false;
        break;
      default:
        filter_.has_password.reset();
        break;
    }
    rebuildRoomCards();
    layoutRoomCards();
    if (on_filter_changed_) {
      on_filter_changed_(filter_);
    }
  };
  addAndMakeVisible(password_filter_combo_);

  auto update_mode_filter = [this]() {
    const bool active = active_filter_toggle_.getToggleState();
    const bool standby = standby_filter_toggle_.getToggleState();
    if (active && !standby) {
      filter_.mode = "active";
    } else if (!active && standby) {
      filter_.mode = "standby";
    } else {
      filter_.mode = "all";
    }
    rebuildRoomCards();
    layoutRoomCards();
    if (on_filter_changed_) {
      on_filter_changed_(filter_);
    }
  };

  active_filter_toggle_.setToggleState(true, juce::dontSendNotification);
  standby_filter_toggle_.setToggleState(false, juce::dontSendNotification);
  filter_.mode = "active";
  active_filter_toggle_.onClick = [this, update_mode_filter] {
    if (active_filter_toggle_.getToggleState()) {
      standby_filter_toggle_.setToggleState(false, juce::dontSendNotification);
    }
    if (!active_filter_toggle_.getToggleState() &&
        !standby_filter_toggle_.getToggleState()) {
      active_filter_toggle_.setToggleState(true, juce::dontSendNotification);
    }
    update_mode_filter();
  };
  standby_filter_toggle_.onClick = [this, update_mode_filter] {
    if (standby_filter_toggle_.getToggleState()) {
      active_filter_toggle_.setToggleState(false, juce::dontSendNotification);
    }
    if (!active_filter_toggle_.getToggleState() &&
        !standby_filter_toggle_.getToggleState()) {
      active_filter_toggle_.setToggleState(true, juce::dontSendNotification);
    }
    update_mode_filter();
  };
  addAndMakeVisible(active_filter_toggle_);
  addAndMakeVisible(standby_filter_toggle_);

  constexpr std::array<const char*, 7> kNavItems = {
      "Rooms", "Profile", "Users", "News", "Manual", "Q&A", "Setting"};
  for (std::size_t i = 0; i < kNavItems.size(); ++i) {
    auto button = std::make_unique<juce::TextButton>(kNavItems[i]);
    button->setClickingTogglesState(true);
    button->setRadioGroupId(1);
    button->setToggleState(i == 0, juce::dontSendNotification);
    button->setTooltip(juce::String("Open ") + juce::String(kNavItems[i]));
    button->setColour(juce::TextButton::buttonColourId,
                      juce::Colour::fromRGB(241, 245, 255));
    button->setColour(juce::TextButton::buttonOnColourId,
                      juce::Colour::fromRGB(78, 107, 223));
    button->setColour(juce::TextButton::textColourOffId,
                      juce::Colour::fromRGB(58, 74, 118));
    button->setColour(juce::TextButton::textColourOnId,
                      tempolink::juceapp::style::TextInverted());
    button->onClick = [this, i] {
      if (!on_navigation_selected_) {
        return;
      }
      switch (i) {
        case 0:
          on_navigation_selected_(NavigationTarget::Rooms);
          break;
        case 1:
          on_navigation_selected_(NavigationTarget::Profile);
          break;
        case 2:
          on_navigation_selected_(NavigationTarget::Users);
          break;
        case 3:
          on_navigation_selected_(NavigationTarget::News);
          break;
        case 4:
          on_navigation_selected_(NavigationTarget::Manual);
          break;
        case 5:
          on_navigation_selected_(NavigationTarget::Qna);
          break;
        case 6:
          on_navigation_selected_(NavigationTarget::Settings);
          break;
        default:
          break;
      }
    };
    addAndMakeVisible(*button);
    nav_buttons_.push_back(std::move(button));
  }

  room_action_fab_.setTooltip("Room action");
  room_action_fab_.setColour(juce::TextButton::buttonColourId,
                             juce::Colour::fromRGB(69, 95, 211));
  room_action_fab_.setColour(juce::TextButton::textColourOffId,
                             tempolink::juceapp::style::TextInverted());
  room_action_fab_.onClick = [this] {
    auto safe_this = juce::Component::SafePointer<LobbyView>(this);
    juce::PopupMenu menu;
    menu.addItem(1, "Join Room");
    menu.addItem(2, "Create Room");
    menu.addItem(3, "My Rooms");
    menu.showMenuAsync(
        juce::PopupMenu::Options().withTargetComponent(room_action_fab_),
        [safe_this](int selected) {
          if (safe_this == nullptr || selected == 0) {
            return;
          }

          if (selected == 1) {
            auto* dialog = new juce::AlertWindow(
                "Join Room", "Enter room code", juce::AlertWindow::NoIcon,
                safe_this.getComponent());
            dialog->addTextEditor("roomCode", "", "Room Code");
            dialog->addButton("Join", 1,
                              juce::KeyPress(juce::KeyPress::returnKey));
            dialog->addButton("Cancel", 0,
                              juce::KeyPress(juce::KeyPress::escapeKey));
            dialog->centreAroundComponent(safe_this.getComponent(), 360, 150);
            dialog->setVisible(true);
            dialog->enterModalState(
                true,
                juce::ModalCallbackFunction::create(
                    [safe_this, dialog](int result) {
                      std::unique_ptr<juce::AlertWindow> cleanup(dialog);
                      if (safe_this == nullptr || result != 1 ||
                          !safe_this->on_enter_room_) {
                        return;
                      }

                      const auto room_code =
                          dialog->getTextEditorContents("roomCode").trim();
                      if (room_code.isNotEmpty()) {
                        safe_this->on_enter_room_(room_code.toStdString());
                      }
                    }),
                false);
            return;
          }

          if (selected == 2) {
            auto* dialog = new juce::AlertWindow(
                "Create Room", "Create a new jam room",
                juce::AlertWindow::NoIcon, safe_this.getComponent());
            juce::StringArray capacity_options;
            for (int i = 2; i <= 6; ++i) {
              capacity_options.add(juce::String(i));
            }
            dialog->addComboBox("capacity", capacity_options, "Max Participants");
            if (auto* combo = dialog->getComboBoxComponent("capacity"); combo != nullptr) {
              combo->setSelectedId(5, juce::dontSendNotification);
            }
            dialog->addButton("Create", 1,
                              juce::KeyPress(juce::KeyPress::returnKey));
            dialog->addButton("Cancel", 0,
                              juce::KeyPress(juce::KeyPress::escapeKey));
            dialog->centreAroundComponent(safe_this.getComponent(), 360, 160);
            dialog->setVisible(true);
            dialog->enterModalState(
                true,
                juce::ModalCallbackFunction::create(
                    [safe_this, dialog](int result) {
                      std::unique_ptr<juce::AlertWindow> cleanup(dialog);
                      if (safe_this == nullptr || result != 1 ||
                          !safe_this->on_create_room_) {
                        return;
                      }

                      int selected_capacity_id = 5;
                      if (auto* combo = dialog->getComboBoxComponent("capacity"); combo != nullptr) {
                        selected_capacity_id = combo->getSelectedId();
                      }
                      const int capacity = juce::jlimit(2, 6, selected_capacity_id + 1);
                      safe_this->on_create_room_("", capacity);
                    }),
                false);
          }

          if (selected == 3 && safe_this->on_open_my_rooms_) {
            safe_this->on_open_my_rooms_();
          }
        });
  };
  addAndMakeVisible(room_action_fab_);

  cards_viewport_.setViewedComponent(&cards_container_, false);
  cards_viewport_.setScrollBarsShown(true, false);
  addAndMakeVisible(cards_viewport_);
  room_action_fab_.toFront(false);
}

void LobbyView::setRooms(const std::vector<RoomSummary>& rooms) {
  rooms_ = rooms;
  rebuildRoomCards();
  resized();
}

void LobbyView::setLogoImage(const juce::Image& logo_image) {
  const bool has_logo = logo_image.isValid();
  logo_image_.setImage(logo_image);
  logo_image_.setVisible(has_logo);
  logo_text_label_.setVisible(!has_logo);
  resized();
}

void LobbyView::setPreviewHandler(
    std::function<void(std::string)> on_preview_room) {
  on_preview_room_ = std::move(on_preview_room);
  for (auto& card : room_cards_) {
    card->setPreviewHandler(on_preview_room_);
  }
}

void LobbyView::setEnterHandler(
    std::function<void(std::string)> on_enter_room) {
  on_enter_room_ = std::move(on_enter_room);
  for (auto& card : room_cards_) {
    card->setEnterHandler(on_enter_room_);
  }
}

void LobbyView::setCreateHandler(
    std::function<void(std::string, int)> on_create_room) {
  on_create_room_ = std::move(on_create_room);
}

void LobbyView::setMyRoomsHandler(std::function<void()> on_open_my_rooms) {
  on_open_my_rooms_ = std::move(on_open_my_rooms);
}

void LobbyView::setNavigationHandler(
    std::function<void(NavigationTarget)> on_navigation_selected) {
  on_navigation_selected_ = std::move(on_navigation_selected);
}

void LobbyView::setFilterChangedHandler(
    std::function<void(const LobbyRoomFilter&)> on_filter_changed) {
  on_filter_changed_ = std::move(on_filter_changed);
  if (on_filter_changed_) {
    on_filter_changed_(filter_);
  }
}

void LobbyView::setStatusText(const juce::String& status_text) {
  status_label_.setText(status_text, juce::dontSendNotification);
}

void LobbyView::resized() {
  auto area = getLocalBounds().reduced(tempolink::juceapp::style::kLobbyPadding);
  auto left = area.removeFromLeft(228);
  area.removeFromLeft(12);
  auto main = area;
  auto left_content = left.reduced(10, 8);

  auto logo_area = left_content.removeFromTop(70);
  if (logo_image_.isVisible()) {
    logo_image_.setBounds(logo_area.reduced(6));
  }
  logo_text_label_.setBounds(logo_area);

  left_content.removeFromTop(8);
  nav_title_label_.setBounds(left_content.removeFromTop(18));
  left_content.removeFromTop(6);

  const int nav_button_height = 34;
  for (auto& button : nav_buttons_) {
    button->setBounds(left_content.removeFromTop(nav_button_height));
    left_content.removeFromTop(6);
  }

  auto header = main.removeFromTop(tempolink::juceapp::style::kLobbyHeaderHeight);
  title_label_.setBounds(header.removeFromLeft(160));
  tabs_label_.setBounds(header.removeFromLeft(250));
  status_label_.setBounds(header);

  main.removeFromTop(6);
  auto filters = main.removeFromTop(28);
  active_filter_toggle_.setBounds(filters.removeFromLeft(120));
  filters.removeFromLeft(8);
  standby_filter_toggle_.setBounds(filters.removeFromLeft(130));
  filters.removeFromLeft(8);
  visibility_filter_combo_.setBounds(filters.removeFromLeft(140));
  filters.removeFromLeft(8);
  password_filter_combo_.setBounds(filters.removeFromLeft(140));
  filters.removeFromLeft(8);
  tag_filter_editor_.setBounds(filters.removeFromLeft(160));
  filters.removeFromLeft(8);
  search_editor_.setBounds(filters);
  main.removeFromTop(8);
  cards_viewport_.setBounds(main);
  auto fab_bounds = cards_viewport_.getBounds().removeFromBottom(56).removeFromRight(56);
  room_action_fab_.setBounds(fab_bounds);
  room_action_fab_.toFront(false);
  layoutRoomCards();
}

void LobbyView::paint(juce::Graphics& g) {
  g.fillAll(tempolink::juceapp::style::LobbyBackground());

  auto bounds = getLocalBounds().reduced(tempolink::juceapp::style::kLobbyPadding);
  auto left = bounds.removeFromLeft(228).toFloat();

  juce::ColourGradient side_gradient(
      juce::Colour::fromRGB(248, 251, 255), left.getTopLeft(),
      juce::Colour::fromRGB(235, 241, 255), left.getBottomLeft(), false);
  g.setGradientFill(side_gradient);
  g.fillRoundedRectangle(left, 14.0F);

  auto nav_section = left.reduced(10.0F, 86.0F);
  g.setColour(juce::Colour::fromRGBA(255, 255, 255, 155));
  g.fillRoundedRectangle(nav_section, 12.0F);

  const auto fab_bounds = room_action_fab_.getBounds().toFloat();
  g.setColour(juce::Colour::fromRGBA(20, 28, 50, 40));
  g.fillEllipse(fab_bounds.translated(2.0F, 3.0F));
}

void LobbyView::rebuildRoomCards() {
  room_cards_.clear();
  cards_container_.removeAllChildren();

  juce::String needle = search_editor_.getText().trim().toLowerCase();
  for (const auto& room : rooms_) {
    if (active_filter_toggle_.getToggleState() && !room.is_active) {
      continue;
    }
    if (standby_filter_toggle_.getToggleState() && room.is_active) {
      continue;
    }
    if (filter_.is_public.has_value() && room.is_public != filter_.is_public.value()) {
      continue;
    }
    if (filter_.has_password.has_value() &&
        room.has_password != filter_.has_password.value()) {
      continue;
    }
    if (!filter_.tag.empty()) {
      bool contains_tag = false;
      for (const auto& tag : room.tags) {
        if (tag.toLowerCase().contains(juce::String(filter_.tag).toLowerCase())) {
          contains_tag = true;
          break;
        }
      }
      if (!contains_tag) {
        continue;
      }
    }
    if (needle.isNotEmpty()) {
      const juce::String haystack =
          (room.title + " " + room.genre + " " + room.host_label + " " +
           room.description)
              .toLowerCase();
      if (!haystack.contains(needle)) {
        continue;
      }
    }

    auto card = std::make_unique<RoomCardComponent>();
    card->setMode(RoomCardComponent::CardMode::Lobby);
    card->setRoom(room);
    card->setPreviewHandler(on_preview_room_);
    card->setEnterHandler(on_enter_room_);
    cards_container_.addAndMakeVisible(*card);
    room_cards_.push_back(std::move(card));
  }
}

void LobbyView::layoutRoomCards() {
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

  cards_container_.setSize(viewport_width, y + card_height + padding);
}
