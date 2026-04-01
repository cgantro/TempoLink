#include "tempolink/juce/ui/session/MyInputPanel.h"

#include <algorithm>
#include <utility>

#include "tempolink/juce/style/UiStyle.h"

MyInputPanel::MyInputPanel() {
  card_.setTitle("My Input");
  addAndMakeVisible(card_);
  card_.setContent(body_);

  input_label_.setText("Input", juce::dontSendNotification);
  output_label_.setText("Output", juce::dontSendNotification);
  input_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());
  output_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());
  body_.addAndMakeVisible(input_label_);
  body_.addAndMakeVisible(output_label_);
  body_.addAndMakeVisible(input_combo_);
  body_.addAndMakeVisible(output_combo_);

  bpm_label_.setText("Metronome BPM", juce::dontSendNotification);
  bpm_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());
  body_.addAndMakeVisible(bpm_label_);

  bpm_slider_.setSliderStyle(juce::Slider::LinearHorizontal);
  bpm_slider_.setRange(30.0, 300.0, 1.0);
  bpm_slider_.setValue(120.0);
  body_.addAndMakeVisible(bpm_slider_);

  body_.addAndMakeVisible(mute_toggle_);
  body_.addAndMakeVisible(metronome_toggle_);
  body_.addAndMakeVisible(audio_settings_button_);
  mute_toggle_.setColour(juce::TextButton::textColourOffId,
                         tempolink::juceapp::style::TextPrimary());
  mute_toggle_.setColour(juce::TextButton::textColourOnId,
                         tempolink::juceapp::style::TextPrimary());
  metronome_toggle_.setColour(juce::TextButton::textColourOffId,
                              tempolink::juceapp::style::TextPrimary());
  metronome_toggle_.setColour(juce::TextButton::textColourOnId,
                              tempolink::juceapp::style::TextPrimary());
  audio_settings_button_.setColour(juce::TextButton::buttonColourId,
                                   tempolink::juceapp::style::CardBackground());
  audio_settings_button_.setColour(juce::TextButton::textColourOffId,
                                   tempolink::juceapp::style::TextPrimary());

  mute_toggle_.onClick = [this] {
    if (!suppress_callbacks_ && on_mute_changed_) {
      on_mute_changed_(mute_toggle_.getToggleState());
    }
  };
  metronome_toggle_.onClick = [this] {
    if (!suppress_callbacks_ && on_metronome_changed_) {
      on_metronome_changed_(metronome_toggle_.getToggleState());
    }
  };
  bpm_slider_.onValueChange = [this] {
    if (!suppress_callbacks_ && on_bpm_changed_) {
      on_bpm_changed_(static_cast<int>(bpm_slider_.getValue()));
    }
  };
  input_combo_.onChange = [this] {
    if (!suppress_callbacks_ && on_input_changed_) {
      on_input_changed_(input_combo_.getText().toStdString());
    }
  };
  output_combo_.onChange = [this] {
    if (!suppress_callbacks_ && on_output_changed_) {
      on_output_changed_(output_combo_.getText().toStdString());
    }
  };
  audio_settings_button_.onClick = [this] {
    if (on_open_audio_settings_) {
      on_open_audio_settings_();
    }
  };
}

void MyInputPanel::setInputDevices(const std::vector<std::string>& devices,
                                   const std::string& selected) {
  suppress_callbacks_ = true;
  input_combo_.clear();
  int id = 1;
  for (const auto& device : devices) {
    input_combo_.addItem(juce::String(device), id++);
  }
  input_combo_.setText(juce::String(selected), juce::dontSendNotification);
  suppress_callbacks_ = false;
}

void MyInputPanel::setOutputDevices(const std::vector<std::string>& devices,
                                    const std::string& selected) {
  suppress_callbacks_ = true;
  output_combo_.clear();
  int id = 1;
  for (const auto& device : devices) {
    output_combo_.addItem(juce::String(device), id++);
  }
  output_combo_.setText(juce::String(selected), juce::dontSendNotification);
  suppress_callbacks_ = false;
}

void MyInputPanel::setMute(bool muted) {
  suppress_callbacks_ = true;
  mute_toggle_.setToggleState(muted, juce::dontSendNotification);
  suppress_callbacks_ = false;
}

void MyInputPanel::setMetronomeEnabled(bool enabled) {
  suppress_callbacks_ = true;
  metronome_toggle_.setToggleState(enabled, juce::dontSendNotification);
  suppress_callbacks_ = false;
}

void MyInputPanel::setMetronomeBpm(int bpm) {
  suppress_callbacks_ = true;
  bpm_slider_.setValue(juce::jlimit(30, 300, bpm), juce::dontSendNotification);
  suppress_callbacks_ = false;
}

void MyInputPanel::setOnMuteChanged(std::function<void(bool)> on_mute_changed) {
  on_mute_changed_ = std::move(on_mute_changed);
}

void MyInputPanel::setOnMetronomeChanged(
    std::function<void(bool)> on_metronome_changed) {
  on_metronome_changed_ = std::move(on_metronome_changed);
}

void MyInputPanel::setOnBpmChanged(std::function<void(int)> on_bpm_changed) {
  on_bpm_changed_ = std::move(on_bpm_changed);
}

void MyInputPanel::setOnInputDeviceChanged(
    std::function<void(std::string)> on_input_changed) {
  on_input_changed_ = std::move(on_input_changed);
}

void MyInputPanel::setOnOutputDeviceChanged(
    std::function<void(std::string)> on_output_changed) {
  on_output_changed_ = std::move(on_output_changed);
}

void MyInputPanel::setOnOpenAudioSettings(std::function<void()> on_open_audio_settings) {
  on_open_audio_settings_ = std::move(on_open_audio_settings);
}

void MyInputPanel::resized() {
  card_.setBounds(getLocalBounds());

  auto area = body_.getLocalBounds();
  input_label_.setBounds(area.removeFromTop(20));
  input_combo_.setBounds(area.removeFromTop(28));
  area.removeFromTop(4);
  output_label_.setBounds(area.removeFromTop(20));
  output_combo_.setBounds(area.removeFromTop(28));
  area.removeFromTop(8);
  mute_toggle_.setBounds(area.removeFromTop(24));
  area.removeFromTop(4);
  metronome_toggle_.setBounds(area.removeFromTop(24));
  area.removeFromTop(6);
  bpm_label_.setBounds(area.removeFromTop(18));
  bpm_slider_.setBounds(area.removeFromTop(24));
  area.removeFromTop(10);
  audio_settings_button_.setBounds(area.removeFromTop(28).removeFromLeft(140));
}
