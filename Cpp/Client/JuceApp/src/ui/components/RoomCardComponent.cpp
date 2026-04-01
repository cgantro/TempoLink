#include "tempolink/juce/ui/components/RoomCardComponent.h"

#include <utility>
#include "tempolink/juce/style/UiStyle.h"

RoomCardComponent::RoomCardComponent() {
  
  title_label_.setFont(juce::FontOptions(17.0F).withStyle("Bold"));
  addAndMakeVisible(title_label_);

  description_label_.setFont(juce::FontOptions(13.0F));
  description_label_.setJustificationType(juce::Justification::centredLeft);
  addAndMakeVisible(description_label_);

  meta_label_.setFont(juce::FontOptions(12.0F));
  addAndMakeVisible(meta_label_);

  host_label_.setFont(juce::FontOptions(12.0F).withStyle("Italic"));
  addAndMakeVisible(host_label_);

  latency_label_.setFont(juce::FontOptions(12.0F).withStyle("Bold"));
  addAndMakeVisible(latency_label_);

  addAndMakeVisible(preview_button_);
  addAndMakeVisible(enter_button_);
  addAndMakeVisible(edit_button_);
  addAndMakeVisible(delete_button_);
  addAndMakeVisible(share_button_);

  preview_button_.onClick = [this] { if (on_preview_) on_preview_(room_.room_code); };
  enter_button_.onClick = [this] { if (on_enter_) on_enter_(room_.room_code); };
  edit_button_.onClick = [this] { if (on_edit_) on_edit_(room_.room_code); };
  delete_button_.onClick = [this] { if (on_delete_) on_delete_(room_.room_code); };
  share_button_.onClick = [this] { if (on_share_) on_share_(room_.room_code); };

  refreshActionVisibility();
  updateTheme();
}
void RoomCardComponent::updateTheme() {
  title_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());
  description_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextSecondary());
  meta_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextSecondary());
  host_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextSecondary());
  latency_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextSecondary());

  auto update_btn = [](juce::TextButton& btn) {
    btn.setColour(juce::TextButton::buttonColourId, tempolink::juceapp::style::CardBackground());
    btn.setColour(juce::TextButton::textColourOffId, tempolink::juceapp::style::TextPrimary());
  };
  update_btn(preview_button_);
  update_btn(enter_button_);
  update_btn(edit_button_);
  update_btn(delete_button_);
  update_btn(share_button_);

  repaint();
}

void RoomCardComponent::setRoom(const RoomSummary& room) {
  room_ = room;
  juce::String tags_text;
  for (std::size_t i = 0; i < room_.tags.size(); ++i) {
    if (i > 0) tags_text << ", ";
    tags_text << room_.tags[i];
  }

  title_label_.setText(room_.title, juce::dontSendNotification);
  description_label_.setText(room_.description.isNotEmpty() ? room_.description : "No description", juce::dontSendNotification);
  
  juce::String meta = room_.genre + "  |  " + juce::String(room_.members) + "/" + juce::String(room_.capacity);
  if (room_.has_password) meta += "  |  Password";
  meta_label_.setText(meta, juce::dontSendNotification);
  
  host_label_.setText("Host: " + room_.host_label, juce::dontSendNotification);
  latency_label_.setText("Tags: " + (tags_text.isNotEmpty() ? tags_text : juce::String("-")) + "  |  " + room_.latency_hint, juce::dontSendNotification);
  
  repaint();
}

void RoomCardComponent::setMode(CardMode mode) {
  mode_ = mode;
  refreshActionVisibility();
}

void RoomCardComponent::setPreviewHandler(std::function<void(std::string)> on_preview) { on_preview_ = std::move(on_preview); }
void RoomCardComponent::setEnterHandler(std::function<void(std::string)> on_enter) { on_enter_ = std::move(on_enter); }
void RoomCardComponent::setEditHandler(std::function<void(std::string)> on_edit) { on_edit_ = std::move(on_edit); }
void RoomCardComponent::setDeleteHandler(std::function<void(std::string)> on_delete) { on_delete_ = std::move(on_delete); }
void RoomCardComponent::setShareHandler(std::function<void(std::string)> on_share) { on_share_ = std::move(on_share); }

void RoomCardComponent::resized() {
  auto area = getLocalBounds().reduced(12);
  title_label_.setBounds(area.removeFromTop(26));
  description_label_.setBounds(area.removeFromTop(20));
  meta_label_.setBounds(area.removeFromTop(22));
  host_label_.setBounds(area.removeFromTop(22));
  latency_label_.setBounds(area.removeFromTop(22));

  auto actions = area.removeFromBottom(30);
  enter_button_.setBounds(actions.removeFromRight(74));
  actions.removeFromRight(6);
  preview_button_.setBounds(actions.removeFromRight(84));
  if (mode_ == CardMode::MyRooms) {
    actions.removeFromRight(8);
    share_button_.setBounds(actions.removeFromRight(66));
    actions.removeFromRight(4);
    delete_button_.setBounds(actions.removeFromRight(66));
    actions.removeFromRight(4);
    edit_button_.setBounds(actions.removeFromRight(66));
  }
}

void RoomCardComponent::paint(juce::Graphics& g) {
  auto bounds = getLocalBounds().toFloat().reduced(1.0F);
  g.setColour(tempolink::juceapp::style::CardBackground());
  g.fillRoundedRectangle(bounds, 14.0F);
  g.setColour(tempolink::juceapp::style::CardBorder());
  g.drawRoundedRectangle(bounds, 14.0F, 1.0F);
}

void RoomCardComponent::refreshActionVisibility() {
  const bool my_rooms = mode_ == CardMode::MyRooms;
  edit_button_.setVisible(my_rooms);
  delete_button_.setVisible(my_rooms);
  share_button_.setVisible(my_rooms);
}
