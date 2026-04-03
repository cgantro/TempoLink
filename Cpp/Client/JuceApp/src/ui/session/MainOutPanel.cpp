#include "tempolink/juce/ui/session/MainOutPanel.h"

#include <algorithm>
#include <utility>

#include "tempolink/juce/style/UiStyle.h"

MainOutPanel::MainOutPanel() {
  card_.setTitle("Main Out");
  addAndMakeVisible(card_);
  card_.setContent(body_);

  master_label_.setText("Master Volume", juce::dontSendNotification);
  master_label_.setFont(juce::FontOptions(13.0F).withStyle("Bold"));
  body_.addAndMakeVisible(master_label_);

  master_slider_.setSliderStyle(juce::Slider::LinearHorizontal);
  master_slider_.setRange(0.0, 1.0, 0.01);
  master_slider_.setValue(1.0);
  master_slider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 24);
  master_slider_.setNumDecimalPlacesToDisplay(2);
  body_.addAndMakeVisible(master_slider_);

  body_.addAndMakeVisible(audio_file_button_);
  audio_file_seek_slider_.setSliderStyle(juce::Slider::LinearHorizontal);
  audio_file_seek_slider_.setRange(0.0, 1.0, 0.001);
  audio_file_seek_slider_.setValue(0.0);
  audio_file_seek_slider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
  body_.addAndMakeVisible(audio_file_seek_slider_);
  body_.addAndMakeVisible(audio_file_loop_toggle_);
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
  audio_file_seek_slider_.onValueChange = [this] {
    if (!suppress_callbacks_ && on_audio_seek_changed_) {
      on_audio_seek_changed_(static_cast<float>(audio_file_seek_slider_.getValue()));
    }
  };
  audio_file_loop_toggle_.onClick = [this] {
    audio_file_loop_enabled_ = audio_file_loop_toggle_.getToggleState();
    if (!suppress_callbacks_ && on_audio_loop_changed_) {
      on_audio_loop_changed_(audio_file_loop_enabled_);
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
  
  updateTheme();
}

void MainOutPanel::updateTheme() {
  master_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());

  master_slider_.setColour(juce::Slider::textBoxTextColourId, tempolink::juceapp::style::TextPrimary());
  master_slider_.setColour(juce::Slider::textBoxBackgroundColourId, tempolink::juceapp::style::CardBackground());
  master_slider_.setColour(juce::Slider::textBoxOutlineColourId, tempolink::juceapp::style::BorderStrong());

  auto update_btn = [](juce::TextButton& btn) {
    btn.setColour(juce::TextButton::buttonColourId, tempolink::juceapp::style::CardBackground());
    btn.setColour(juce::TextButton::textColourOffId, tempolink::juceapp::style::TextPrimary());
  };
  update_btn(audio_file_button_);
  update_btn(record_button_);
  update_btn(disconnect_button_);
  audio_file_loop_toggle_.setColour(juce::ToggleButton::textColourId,
                                    tempolink::juceapp::style::TextPrimary());
  audio_file_seek_slider_.setColour(juce::Slider::thumbColourId,
                                    tempolink::juceapp::style::AccentCyan());
  audio_file_seek_slider_.setColour(juce::Slider::trackColourId,
                                    tempolink::juceapp::style::PanelBorder());

  repaint();
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
  if (!audio_file_active_) {
    audio_file_seek_slider_.setValue(0.0, juce::dontSendNotification);
  }
  suppress_callbacks_ = false;
}

void MainOutPanel::setAudioFilePlaybackPosition(float normalized_position) {
  suppress_callbacks_ = true;
  audio_file_seek_slider_.setValue(std::clamp(normalized_position, 0.0F, 1.0F),
                                   juce::dontSendNotification);
  suppress_callbacks_ = false;
}

void MainOutPanel::setAudioFileLoopEnabled(bool enabled) {
  suppress_callbacks_ = true;
  audio_file_loop_enabled_ = enabled;
  audio_file_loop_toggle_.setToggleState(enabled, juce::dontSendNotification);
  suppress_callbacks_ = false;
}

void MainOutPanel::setOnVolumeChanged(std::function<void(float)> on_volume_changed) {
  on_volume_changed_ = std::move(on_volume_changed);
}

void MainOutPanel::setOnAudioFileToggle(
    std::function<void(bool)> on_audio_file_toggle) {
  on_audio_file_toggle_ = std::move(on_audio_file_toggle);
}

void MainOutPanel::setOnAudioFileSeekChanged(
    std::function<void(float)> on_audio_seek_changed) {
  on_audio_seek_changed_ = std::move(on_audio_seek_changed);
}

void MainOutPanel::setOnAudioFileLoopChanged(
    std::function<void(bool)> on_audio_loop_changed) {
  on_audio_loop_changed_ = std::move(on_audio_loop_changed);
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
  area.removeFromTop(6);
  audio_file_seek_slider_.setBounds(area.removeFromTop(20));
  area.removeFromTop(4);
  auto row = area.removeFromTop(30);
  audio_file_button_.setBounds(row.removeFromLeft(110));
  row.removeFromLeft(8);
  audio_file_loop_toggle_.setBounds(row.removeFromLeft(76));
  row.removeFromLeft(8);
  record_button_.setBounds(row.removeFromLeft(100));
  row.removeFromLeft(8);
  disconnect_button_.setBounds(row.removeFromLeft(80));
}
