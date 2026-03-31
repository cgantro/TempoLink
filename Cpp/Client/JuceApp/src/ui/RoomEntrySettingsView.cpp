#include "tempolink/juce/ui/RoomEntrySettingsView.h"

#include <utility>

#include "tempolink/juce/style/UiStyle.h"

RoomEntrySettingsView::RoomEntrySettingsView() {
  card_.setTitle("Room Entry");
  addAndMakeVisible(card_);
  card_.setContent(body_);

  title_label_.setFont(juce::FontOptions(24.0F).withStyle("Bold"));
  title_label_.setColour(juce::Label::textColourId,
                         tempolink::juceapp::style::TextPrimary());
  title_label_.setText("Entry Settings", juce::dontSendNotification);
  body_.addAndMakeVisible(title_label_);

  room_label_.setColour(juce::Label::textColourId,
                        tempolink::juceapp::style::TextSecondary());
  room_label_.setText("Room: -", juce::dontSendNotification);
  body_.addAndMakeVisible(room_label_);

  part_label_.setText("Part", juce::dontSendNotification);
  input_label_.setText("Input Device", juce::dontSendNotification);
  output_label_.setText("Output Device", juce::dontSendNotification);
  part_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());
  input_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());
  output_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());
  body_.addAndMakeVisible(part_label_);
  body_.addAndMakeVisible(input_label_);
  body_.addAndMakeVisible(output_label_);

  PopulatePartOptions(part_combo_);
  body_.addAndMakeVisible(part_combo_);
  body_.addAndMakeVisible(input_combo_);
  body_.addAndMakeVisible(output_combo_);

  preflight_label_.setColour(juce::Label::textColourId,
                             tempolink::juceapp::style::TextSecondary());
  body_.addAndMakeVisible(preflight_label_);

  status_label_.setColour(juce::Label::textColourId,
                          tempolink::juceapp::style::TextSecondary());
  status_label_.setText("Select part and devices before entering.", juce::dontSendNotification);
  body_.addAndMakeVisible(status_label_);

  body_.addAndMakeVisible(back_button_);
  body_.addAndMakeVisible(audio_settings_button_);
  body_.addAndMakeVisible(join_button_);

  part_combo_.onChange = [this] {
    updatePreflightState();
    if (!suppress_callbacks_ && on_part_changed_) {
      on_part_changed_(part_combo_.getText().toStdString());
    }
  };
  input_combo_.onChange = [this] {
    updatePreflightState();
    if (!suppress_callbacks_ && on_input_changed_) {
      on_input_changed_(input_combo_.getText().toStdString());
    }
  };
  output_combo_.onChange = [this] {
    updatePreflightState();
    if (!suppress_callbacks_ && on_output_changed_) {
      on_output_changed_(output_combo_.getText().toStdString());
    }
  };
  back_button_.onClick = [this] {
    if (on_back_) {
      on_back_();
    }
  };
  audio_settings_button_.onClick = [this] {
    if (on_open_audio_settings_) {
      on_open_audio_settings_();
    }
  };
  join_button_.onClick = [this] {
    if (on_join_) {
      on_join_(currentSelection());
    }
  };

  updatePreflightState();
}

void RoomEntrySettingsView::setRoom(const RoomSummary& room) {
  room_ = room;
  room_label_.setText("Room: " + room_.title + "  (" + room_.host_label + ")",
                      juce::dontSendNotification);
}

void RoomEntrySettingsView::setStatusText(const juce::String& status_text) {
  status_label_.setText(status_text, juce::dontSendNotification);
}

void RoomEntrySettingsView::setInputDevices(const std::vector<std::string>& devices,
                                            const std::string& selected_device) {
  suppress_callbacks_ = true;
  input_combo_.clear();
  int id = 1;
  for (const auto& device : devices) {
    input_combo_.addItem(juce::String(device), id++);
  }
  input_combo_.setText(juce::String(selected_device), juce::dontSendNotification);
  suppress_callbacks_ = false;
  updatePreflightState();
}

void RoomEntrySettingsView::setOutputDevices(const std::vector<std::string>& devices,
                                             const std::string& selected_device) {
  suppress_callbacks_ = true;
  output_combo_.clear();
  int id = 1;
  for (const auto& device : devices) {
    output_combo_.addItem(juce::String(device), id++);
  }
  output_combo_.setText(juce::String(selected_device), juce::dontSendNotification);
  suppress_callbacks_ = false;
  updatePreflightState();
}

void RoomEntrySettingsView::setSelectedPart(const std::string& part_label) {
  suppress_callbacks_ = true;
  part_combo_.setText(juce::String(part_label), juce::dontSendNotification);
  suppress_callbacks_ = false;
  updatePreflightState();
}

RoomEntrySettingsView::EntrySelection RoomEntrySettingsView::currentSelection() const {
  EntrySelection selection;
  selection.room_code = room_.room_code;
  selection.part_label = part_combo_.getText().toStdString();
  selection.input_device = input_combo_.getText().toStdString();
  selection.output_device = output_combo_.getText().toStdString();
  return selection;
}

void RoomEntrySettingsView::setOnBack(std::function<void()> on_back) {
  on_back_ = std::move(on_back);
}

void RoomEntrySettingsView::setOnJoin(
    std::function<void(EntrySelection)> on_join) {
  on_join_ = std::move(on_join);
}

void RoomEntrySettingsView::setOnPartChanged(
    std::function<void(std::string)> on_part_changed) {
  on_part_changed_ = std::move(on_part_changed);
}

void RoomEntrySettingsView::setOnInputDeviceChanged(
    std::function<void(std::string)> on_input_changed) {
  on_input_changed_ = std::move(on_input_changed);
}

void RoomEntrySettingsView::setOnOutputDeviceChanged(
    std::function<void(std::string)> on_output_changed) {
  on_output_changed_ = std::move(on_output_changed);
}

void RoomEntrySettingsView::setOnOpenAudioSettings(
    std::function<void()> on_open_audio_settings) {
  on_open_audio_settings_ = std::move(on_open_audio_settings);
}

void RoomEntrySettingsView::resized() {
  card_.setBounds(getLocalBounds().reduced(20));

  auto area = body_.getLocalBounds().reduced(10);
  title_label_.setBounds(area.removeFromTop(34));
  room_label_.setBounds(area.removeFromTop(24));
  area.removeFromTop(8);

  part_label_.setBounds(area.removeFromTop(20));
  part_combo_.setBounds(area.removeFromTop(28));
  area.removeFromTop(6);

  input_label_.setBounds(area.removeFromTop(20));
  input_combo_.setBounds(area.removeFromTop(28));
  area.removeFromTop(6);

  output_label_.setBounds(area.removeFromTop(20));
  output_combo_.setBounds(area.removeFromTop(28));
  area.removeFromTop(10);

  preflight_label_.setBounds(area.removeFromTop(20));
  area.removeFromTop(4);
  status_label_.setBounds(area.removeFromTop(22));
  area.removeFromTop(12);

  auto actions = area.removeFromTop(30);
  back_button_.setBounds(actions.removeFromLeft(90));
  actions.removeFromLeft(8);
  audio_settings_button_.setBounds(actions.removeFromLeft(140));
  actions.removeFromLeft(8);
  join_button_.setBounds(actions.removeFromLeft(140));
}

void RoomEntrySettingsView::paint(juce::Graphics& g) {
  g.fillAll(tempolink::juceapp::style::SessionBackground());
}

void RoomEntrySettingsView::updatePreflightState() {
  const bool has_part = part_combo_.getText().trim().isNotEmpty();
  const bool has_input = input_combo_.getText().trim().isNotEmpty();
  const bool has_output = output_combo_.getText().trim().isNotEmpty();

  juce::String checks;
  checks += has_part ? "Part OK" : "Part Missing";
  checks += " | ";
  checks += has_input ? "Input OK" : "Input Missing";
  checks += " | ";
  checks += has_output ? "Output OK" : "Output Missing";
  preflight_label_.setText(checks, juce::dontSendNotification);

  const bool ready = has_part && has_input && has_output;
  join_button_.setEnabled(ready);
}

void RoomEntrySettingsView::PopulatePartOptions(juce::ComboBox& combo_box) {
  combo_box.addItem("Vocals", 1);
  combo_box.addItem("Guitar", 2);
  combo_box.addItem("Bass", 3);
  combo_box.addItem("Drums", 4);
  combo_box.addItem("Keys", 5);
  combo_box.addItem("Other", 6);
  combo_box.setSelectedId(2, juce::dontSendNotification);
}
