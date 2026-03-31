#include "tempolink/juce/ui/LobbyView.h"

#include <algorithm>
#include <utility>

#include "tempolink/juce/style/UiStyle.h"

LobbyView::LobbyView() {
  title_label_.setText("Rooms", juce::dontSendNotification);
  title_label_.setFont(juce::FontOptions(24.0F).withStyle("Bold"));
  title_label_.setColour(juce::Label::textColourId,
                         tempolink::juceapp::style::TextPrimary());
  addAndMakeVisible(title_label_);

  status_label_.setText("Select a room and join session.", juce::dontSendNotification);
  status_label_.setColour(juce::Label::textColourId,
                          tempolink::juceapp::style::TextSecondary());
  addAndMakeVisible(status_label_);

  search_editor_.setTextToShowWhenEmpty("Search rooms",
                                        tempolink::juceapp::style::TextSecondary());
  search_editor_.setColour(juce::TextEditor::textColourId,
                           tempolink::juceapp::style::TextPrimary());
  search_editor_.setColour(juce::TextEditor::backgroundColourId,
                           tempolink::juceapp::style::PanelBackground());
  search_editor_.setColour(juce::TextEditor::outlineColourId,
                           tempolink::juceapp::style::BorderStrong());
  search_editor_.onTextChange = [this] {
    rebuildRoomCards();
    layoutRoomCards();
  };
  addAndMakeVisible(search_editor_);

  cards_viewport_.setViewedComponent(&cards_container_, false);
  cards_viewport_.setScrollBarsShown(true, false);
  addAndMakeVisible(cards_viewport_);
}

void LobbyView::setRooms(const std::vector<RoomSummary>& rooms) {
  rooms_ = rooms;
  rebuildRoomCards();
  resized();
}

void LobbyView::setJoinHandler(
    std::function<void(std::string)> on_join_room) {
  on_join_room_ = std::move(on_join_room);
  for (auto& card : room_cards_) {
    card->setJoinHandler(on_join_room_);
  }
}

void LobbyView::setStatusText(const juce::String& status_text) {
  status_label_.setText(status_text, juce::dontSendNotification);
}

void LobbyView::resized() {
  auto area = getLocalBounds().reduced(tempolink::juceapp::style::kLobbyPadding);
  auto header = area.removeFromTop(tempolink::juceapp::style::kLobbyHeaderHeight);
  title_label_.setBounds(header.removeFromLeft(160));
  status_label_.setBounds(header);

  area.removeFromTop(8);
  search_editor_.setBounds(
      area.removeFromTop(30).removeFromRight(tempolink::juceapp::style::kLobbySearchWidth));
  area.removeFromTop(8);
  cards_viewport_.setBounds(area);
  layoutRoomCards();
}

void LobbyView::paint(juce::Graphics& g) {
  g.fillAll(tempolink::juceapp::style::LobbyBackground());
}

void LobbyView::rebuildRoomCards() {
  room_cards_.clear();
  cards_container_.removeAllChildren();

  juce::String needle = search_editor_.getText().trim().toLowerCase();
  for (const auto& room : rooms_) {
    if (needle.isNotEmpty()) {
      const juce::String haystack =
          (room.title + " " + room.genre + " " + room.host_label).toLowerCase();
      if (!haystack.contains(needle)) {
        continue;
      }
    }

    auto card = std::make_unique<RoomCardComponent>();
    card->setRoom(room);
    card->setJoinHandler(on_join_room_);
    cards_container_.addAndMakeVisible(*card);
    room_cards_.push_back(std::move(card));
  }
}

void LobbyView::layoutRoomCards() {
  if (cards_viewport_.getWidth() <= 0) {
    return;
  }

  const int viewport_width = cards_viewport_.getWidth();
  const int padding = tempolink::juceapp::style::kLobbyGridPadding;
  const int column_count = viewport_width > 980 ? 3 : (viewport_width > 680 ? 2 : 1);
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
