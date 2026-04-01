#include "tempolink/juce/ui/session/MainOutPanel.h"

#include <algorithm>
#include <utility>

#include "tempolink/juce/style/UiStyle.h"

MainOutPanel::MainOutPanel() {
  card_.setTitle("Main Out");
  addAndMakeVisible(card_);
  card_.setContent(body_);

  master_label_.setText("Master Volume", juce::dontSendNotification);
  master_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());
  master_label_.setFont(juce::FontOptions(13.0F).withStyle("Bold"));
  body_.addAndMakeVisible(master_label_);

  master_slider_.setSliderStyle(juce::Slider::LinearHorizontal);
  master_slider_.setRange(0.0, 1.0, 0.01);
  master_slider_.setValue(1.0);
  master_slider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 28);
  master_slider_.setNumDecimalPlacesToDisplay(2);
  master_slider_.setColour(juce::Slider::textBoxTextColourId,
                           tempolink::juceapp::style::TextPrimary());
  master_slider_.setColour(juce::Slider::textBoxBackgroundColourId,
                           tempolink::juceapp::style::CardBackground());
  master_slider_.setColour(juce::Slider::textBoxOutlineColourId,
                           tempolink::juceapp::style::BorderStrong());
  body_.addAndMakeVisible(master_slider_);

  audio_file_button_.setColour(juce::TextButton::buttonColourId,
                               tempolink::juceapp::style::CardBackground());
  audio_file_button_.setColour(juce::TextButton::textColourOffId,
                               tempolink::juceapp::style::TextPrimary());
  body_.addAndMakeVisible(audio_file_button_);

  record_button_.setColour(juce::TextButton::buttonColourId,
                           tempolink::juceapp::style::CardBackground());
  record_button_.setColour(juce::TextButton::textColourOffId,
                           tempolink::juceapp::style::TextPrimary());
  body_.addAndMakeVisible(record_button_);

  body_.addAndMakeVisible(disconnect_button_);

  master_slider_.onValueChange = [this] {
    if (!suppress_callbacks_ && on_volume_changed_) {
      on_volume_changed_(static_cast<float>(master_slider_.getValue()));
    }
  };
  audio_file_button_.onClick = [this] {
    audio_file_active_ = !audio_file_active_;
    audio_file_button_.setButtonText(audio_file_active_ ? "Audio Stop"
                                                         : "Audio File");
    if (!suppress_callbacks_ && on_audio_file_toggle_) {
      on_audio_file_toggle_(audio_file_active_);
    }
  };
  record_button_.onClick = [this] {
    recording_ = !recording_;
    record_button_.setButtonText(recording_ ? "REC ON" : "Record");
    if (!suppress_callbacks_ && on_record_toggle_) {
      on_record_toggle_(recording_);
    }
  };
  disconnect_button_.onClick = [this] {
    if (on_disconnect_) {
      on_disconnect_();
    }
  };
}

void MainOutPanel::setMasterVolume(float volume) {
  suppress_callbacks_ = true;
  master_slider_.setValue(std::clamp(volume, 0.0F, 1.0F), juce::dontSendNotification);
  suppress_callbacks_ = false;
}

void MainOutPanel::setRecording(bool recording) {
  suppress_callbacks_ = true;
  recording_ = recording;
  record_button_.setButtonText(recording_ ? "REC ON" : "Record");
  suppress_callbacks_ = false;
}

void MainOutPanel::setAudioFileActive(bool active) {
  suppress_callbacks_ = true;
  audio_file_active_ = active;
  audio_file_button_.setButtonText(audio_file_active_ ? "Audio Stop"
                                                       : "Audio File");
  suppress_callbacks_ = false;
}

void MainOutPanel::setOnVolumeChanged(std::function<void(float)> on_volume_changed) {
  on_volume_changed_ = std::move(on_volume_changed);
}

void MainOutPanel::setOnAudioFileToggle(
    std::function<void(bool)> on_audio_file_toggle) {
  on_audio_file_toggle_ = std::move(on_audio_file_toggle);
}

void MainOutPanel::setOnRecordToggle(std::function<void(bool)> on_record_toggle) {
  on_record_toggle_ = std::move(on_record_toggle);
}

void MainOutPanel::setOnDisconnect(std::function<void()> on_disconnect) {
  on_disconnect_ = std::move(on_disconnect);
}

void MainOutPanel::resized() {
  card_.setBounds(getLocalBounds());
  auto area = body_.getLocalBounds();
  master_label_.setBounds(area.removeFromTop(20));
  master_slider_.setBounds(area.removeFromTop(30));
  area.removeFromTop(10);
  auto row = area.removeFromTop(30);
  audio_file_button_.setBounds(row.removeFromLeft(110));
  row.removeFromLeft(8);
  record_button_.setBounds(row.removeFromLeft(100));
  row.removeFromLeft(8);
  disconnect_button_.setBounds(row.removeFromLeft(80));
}
