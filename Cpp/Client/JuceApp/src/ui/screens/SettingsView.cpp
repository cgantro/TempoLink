#include "tempolink/juce/ui/screens/SettingsView.h"

#include <utility>

#include "tempolink/juce/constants/AudioSettingsConstants.h"
#include "tempolink/juce/style/UiStyle.h"

SettingsView::SettingsView() {
  back_button_.onClick = [this] {
    if (on_back_) {
      on_back_();
    }
  };
  addAndMakeVisible(back_button_);

  title_label_.setText("Settings", juce::dontSendNotification);
  title_label_.setFont(juce::FontOptions(26.0F).withStyle("Bold"));
  title_label_.setColour(juce::Label::textColourId,
                         tempolink::juceapp::style::TextPrimary());
  addAndMakeVisible(title_label_);

  status_label_.setColour(juce::Label::textColourId,
                          tempolink::juceapp::style::TextSecondary());
  addAndMakeVisible(status_label_);

  auto init_label = [](juce::Label& label) {
    label.setColour(juce::Label::textColourId,
                    tempolink::juceapp::style::TextPrimary());
  };
  init_label(input_label_);
  init_label(output_label_);
  init_label(sample_rate_label_);
  init_label(buffer_label_);
  init_label(nickname_label_);
  init_label(bio_label_);
  addAndMakeVisible(input_label_);
  addAndMakeVisible(output_label_);
  addAndMakeVisible(sample_rate_label_);
  addAndMakeVisible(buffer_label_);
  addAndMakeVisible(nickname_label_);
  addAndMakeVisible(bio_label_);

  input_combo_.onChange = [this] {
    if (on_input_changed_) {
      on_input_changed_(input_combo_.getText().toStdString());
    }
  };
  output_combo_.onChange = [this] {
    if (on_output_changed_) {
      on_output_changed_(output_combo_.getText().toStdString());
    }
  };
  addAndMakeVisible(input_combo_);
  addAndMakeVisible(output_combo_);

  int id = 1;
  for (int sample_rate : tempolink::juceapp::constants::kSupportedSampleRates) {
    sample_rate_combo_.addItem(juce::String(sample_rate), id++);
  }
  addAndMakeVisible(sample_rate_combo_);

  id = 1;
  for (int buffer_samples : tempolink::juceapp::constants::kSupportedBufferSamples) {
    buffer_combo_.addItem(juce::String(buffer_samples), id++);
  }
  addAndMakeVisible(buffer_combo_);

  nickname_editor_.setTextToShowWhenEmpty("nickname", tempolink::juceapp::style::TextSecondary());
  nickname_editor_.setColour(juce::TextEditor::textColourId,
                             tempolink::juceapp::style::TextPrimary());
  nickname_editor_.setColour(juce::TextEditor::backgroundColourId,
                             tempolink::juceapp::style::PanelBackground());
  nickname_editor_.setColour(juce::TextEditor::outlineColourId,
                             tempolink::juceapp::style::BorderStrong());
  addAndMakeVisible(nickname_editor_);

  bio_editor_.setMultiLine(true);
  bio_editor_.setReturnKeyStartsNewLine(true);
  bio_editor_.setTextToShowWhenEmpty("bio", tempolink::juceapp::style::TextSecondary());
  bio_editor_.setColour(juce::TextEditor::textColourId,
                        tempolink::juceapp::style::TextPrimary());
  bio_editor_.setColour(juce::TextEditor::backgroundColourId,
                        tempolink::juceapp::style::PanelBackground());
  bio_editor_.setColour(juce::TextEditor::outlineColourId,
                        tempolink::juceapp::style::BorderStrong());
  addAndMakeVisible(bio_editor_);

  apply_button_.onClick = [this] {
    if (!on_apply_audio_) {
      return;
    }
    const int sample_rate = sample_rate_combo_.getText().getIntValue();
    const int buffer_samples = buffer_combo_.getText().getIntValue();
    if (sample_rate > 0 && buffer_samples > 0) {
      on_apply_audio_(sample_rate, buffer_samples);
    }
  };
  apply_button_.setColour(juce::TextButton::buttonColourId,
                          tempolink::juceapp::style::CardBackground());
  apply_button_.setColour(juce::TextButton::textColourOffId,
                          tempolink::juceapp::style::TextPrimary());
  addAndMakeVisible(apply_button_);

  save_profile_button_.onClick = [this] {
    if (!on_save_profile_) {
      return;
    }
    on_save_profile_(nickname_editor_.getText().trim().toStdString(),
                     bio_editor_.getText().trim().toStdString());
  };
  save_profile_button_.setColour(juce::TextButton::buttonColourId,
                                 tempolink::juceapp::style::CardBackground());
  save_profile_button_.setColour(juce::TextButton::textColourOffId,
                                 tempolink::juceapp::style::TextPrimary());
  addAndMakeVisible(save_profile_button_);
}

void SettingsView::setOnBack(std::function<void()> on_back) {
  on_back_ = std::move(on_back);
}

void SettingsView::setOnInputDeviceChanged(
    std::function<void(std::string)> on_input_changed) {
  on_input_changed_ = std::move(on_input_changed);
}

void SettingsView::setOnOutputDeviceChanged(
    std::function<void(std::string)> on_output_changed) {
  on_output_changed_ = std::move(on_output_changed);
}

void SettingsView::setOnApplyAudio(std::function<void(int, int)> on_apply_audio) {
  on_apply_audio_ = std::move(on_apply_audio);
}

void SettingsView::setOnSaveProfile(
    std::function<void(std::string, std::string)> on_save_profile) {
  on_save_profile_ = std::move(on_save_profile);
}

void SettingsView::setDevices(const std::vector<std::string>& input_devices,
                              const std::string& selected_input,
                              const std::vector<std::string>& output_devices,
                              const std::string& selected_output) {
  input_combo_.clear(juce::dontSendNotification);
  output_combo_.clear(juce::dontSendNotification);

  int id = 1;
  int selected_input_id = 0;
  for (const auto& device : input_devices) {
    input_combo_.addItem(device, id);
    if (device == selected_input) {
      selected_input_id = id;
    }
    ++id;
  }
  id = 1;
  int selected_output_id = 0;
  for (const auto& device : output_devices) {
    output_combo_.addItem(device, id);
    if (device == selected_output) {
      selected_output_id = id;
    }
    ++id;
  }

  if (selected_input_id > 0) {
    input_combo_.setSelectedId(selected_input_id, juce::dontSendNotification);
  } else if (!input_devices.empty()) {
    input_combo_.setSelectedId(1, juce::dontSendNotification);
  }

  if (selected_output_id > 0) {
    output_combo_.setSelectedId(selected_output_id, juce::dontSendNotification);
  } else if (!output_devices.empty()) {
    output_combo_.setSelectedId(1, juce::dontSendNotification);
  }
}

void SettingsView::setAudioFormat(int sample_rate_hz, int buffer_samples) {
  sample_rate_combo_.setText(juce::String(sample_rate_hz),
                             juce::dontSendNotification);
  buffer_combo_.setText(juce::String(buffer_samples), juce::dontSendNotification);
}

void SettingsView::setProfileFields(const std::string& display_name,
                                    const std::string& bio) {
  nickname_editor_.setText(juce::String(display_name), juce::dontSendNotification);
  bio_editor_.setText(juce::String(bio), juce::dontSendNotification);
}

void SettingsView::setStatusText(const juce::String& status_text) {
  status_label_.setText(status_text, juce::dontSendNotification);
}

void SettingsView::resized() {
  auto area = getLocalBounds().reduced(18);
  auto top = area.removeFromTop(60);
  back_button_.setBounds(top.removeFromLeft(120).reduced(0, 14));
  top.removeFromLeft(8);
  title_label_.setBounds(top.removeFromLeft(240));
  status_label_.setBounds(area.removeFromTop(28));
  area.removeFromTop(8);

  auto row1 = area.removeFromTop(32);
  input_label_.setBounds(row1.removeFromLeft(110));
  input_combo_.setBounds(row1.removeFromLeft(260));
  row1.removeFromLeft(20);
  output_label_.setBounds(row1.removeFromLeft(110));
  output_combo_.setBounds(row1.removeFromLeft(260));

  area.removeFromTop(8);
  auto row2 = area.removeFromTop(32);
  sample_rate_label_.setBounds(row2.removeFromLeft(110));
  sample_rate_combo_.setBounds(row2.removeFromLeft(180));
  row2.removeFromLeft(20);
  buffer_label_.setBounds(row2.removeFromLeft(80));
  buffer_combo_.setBounds(row2.removeFromLeft(140));
  row2.removeFromLeft(20);
  apply_button_.setBounds(row2.removeFromLeft(100));
  row2.removeFromLeft(10);
  save_profile_button_.setBounds(row2.removeFromLeft(130));

  area.removeFromTop(10);
  auto profile_row = area.removeFromTop(32);
  nickname_label_.setBounds(profile_row.removeFromLeft(110));
  nickname_editor_.setBounds(profile_row.removeFromLeft(320));

  area.removeFromTop(8);
  auto bio_row = area.removeFromTop(120);
  bio_label_.setBounds(bio_row.removeFromLeft(110));
  bio_editor_.setBounds(bio_row);
}

void SettingsView::paint(juce::Graphics& g) {
  g.fillAll(tempolink::juceapp::style::LobbyBackground());
}
