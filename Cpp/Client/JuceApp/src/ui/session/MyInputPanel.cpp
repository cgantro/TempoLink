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
  body_.addAndMakeVisible(input_label_);
  body_.addAndMakeVisible(output_label_);
  body_.addAndMakeVisible(input_combo_);
  body_.addAndMakeVisible(output_combo_);

  input_level_label_.setText("Input Level", juce::dontSendNotification);
  input_level_label_.setFont(juce::FontOptions(13.0F).withStyle("Bold"));
  body_.addAndMakeVisible(input_level_label_);
  body_.addAndMakeVisible(level_meter_);

  input_gain_label_.setText("Input Gain", juce::dontSendNotification);
  input_gain_label_.setFont(juce::FontOptions(13.0F).withStyle("Bold"));
  body_.addAndMakeVisible(input_gain_label_);

  input_gain_slider_.setSliderStyle(juce::Slider::LinearHorizontal);
  input_gain_slider_.setRange(0.0, 2.0, 0.01);
  input_gain_slider_.setValue(1.0);
  input_gain_slider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 24);
  input_gain_slider_.setNumDecimalPlacesToDisplay(2);
  input_gain_slider_.setTextValueSuffix(" x");
  body_.addAndMakeVisible(input_gain_slider_);

  reverb_label_.setText("Reverb", juce::dontSendNotification);
  reverb_label_.setFont(juce::FontOptions(13.0F).withStyle("Bold"));
  body_.addAndMakeVisible(reverb_label_);

  reverb_slider_.setSliderStyle(juce::Slider::LinearHorizontal);
  reverb_slider_.setRange(0.0, 1.0, 0.01);
  reverb_slider_.setValue(0.0);
  reverb_slider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 24);
  reverb_slider_.setNumDecimalPlacesToDisplay(2);
  reverb_slider_.setTextValueSuffix(" mix");
  body_.addAndMakeVisible(reverb_slider_);

  bpm_label_.setText("Metronome BPM", juce::dontSendNotification);
  bpm_label_.setFont(juce::FontOptions(13.0F).withStyle("Bold"));
  body_.addAndMakeVisible(bpm_label_);

  bpm_slider_.setSliderStyle(juce::Slider::LinearHorizontal);
  bpm_slider_.setRange(30.0, 300.0, 1.0);
  bpm_slider_.setValue(120.0);
  bpm_slider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 84, 24);
  bpm_slider_.setNumDecimalPlacesToDisplay(0);
  bpm_slider_.setTextValueSuffix(" BPM");
  body_.addAndMakeVisible(bpm_slider_);
  metronome_tone_label_.setText("Metronome Tone", juce::dontSendNotification);
  metronome_tone_label_.setFont(juce::FontOptions(13.0F).withStyle("Bold"));
  body_.addAndMakeVisible(metronome_tone_label_);
  metronome_tone_combo_.addItem("Tone 1", 1);
  metronome_tone_combo_.addItem("Tone 2", 2);
  metronome_tone_combo_.addItem("Tone 3", 3);
  metronome_tone_combo_.setSelectedId(1, juce::dontSendNotification);
  body_.addAndMakeVisible(metronome_tone_combo_);
  body_.addAndMakeVisible(bpm_minus_button_);
  body_.addAndMakeVisible(bpm_plus_button_);

  body_.addAndMakeVisible(mute_toggle_);
  body_.addAndMakeVisible(metronome_toggle_);
  body_.addAndMakeVisible(audio_settings_button_);

  mute_toggle_.setClickingTogglesState(true);
  mute_toggle_.setToggleState(false, juce::dontSendNotification);
  metronome_toggle_.setClickingTogglesState(true);
  metronome_toggle_.setToggleState(false, juce::dontSendNotification);

  mute_toggle_.onClick = [this] {
    if (!suppress_callbacks_ && on_mute_changed_) {
      on_mute_changed_(mute_toggle_.getToggleState());
    }
  };
  input_gain_slider_.onValueChange = [this] {
    if (!suppress_callbacks_ && on_input_gain_changed_) {
      on_input_gain_changed_(static_cast<float>(input_gain_slider_.getValue()));
    }
  };
  reverb_slider_.onValueChange = [this] {
    if (!suppress_callbacks_ && on_input_reverb_changed_) {
      on_input_reverb_changed_(static_cast<float>(reverb_slider_.getValue()));
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
  metronome_tone_combo_.onChange = [this] {
    if (!suppress_callbacks_ && on_metronome_tone_changed_) {
      on_metronome_tone_changed_(juce::jmax(0, metronome_tone_combo_.getSelectedId() - 1));
    }
  };
  bpm_minus_button_.onClick = [this] {
    if (suppress_callbacks_) {
      return;
    }
    bpm_slider_.setValue(bpm_slider_.getValue() - 1.0);
  };
  bpm_plus_button_.onClick = [this] {
    if (suppress_callbacks_) {
      return;
    }
    bpm_slider_.setValue(bpm_slider_.getValue() + 1.0);
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

  updateTheme();
}

void MyInputPanel::updateTheme() {
  input_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());
  output_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());
  input_level_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());
  input_gain_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());
  reverb_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());
  bpm_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());
  metronome_tone_label_.setColour(juce::Label::textColourId,
                                  tempolink::juceapp::style::TextPrimary());

  auto update_sl = [](juce::Slider& sl) {
    sl.setColour(juce::Slider::textBoxTextColourId, tempolink::juceapp::style::TextPrimary());
    sl.setColour(juce::Slider::textBoxBackgroundColourId, tempolink::juceapp::style::CardBackground());
    sl.setColour(juce::Slider::textBoxOutlineColourId, tempolink::juceapp::style::BorderStrong());
  };
  update_sl(input_gain_slider_);
  update_sl(reverb_slider_);
  update_sl(bpm_slider_);

  auto update_toggle = [](juce::TextButton& btn) {
    btn.setColour(juce::TextButton::buttonColourId,
                  tempolink::juceapp::style::CardBackground());
    btn.setColour(juce::TextButton::buttonOnColourId,
                  tempolink::juceapp::style::AccentCyan().withAlpha(0.3F));
    btn.setColour(juce::TextButton::textColourOffId,
                  tempolink::juceapp::style::TextPrimary());
    btn.setColour(juce::TextButton::textColourOnId,
                  tempolink::juceapp::style::TextPrimary());
  };
  update_toggle(mute_toggle_);
  update_toggle(metronome_toggle_);

  auto update_btn = [](juce::TextButton& btn) {
    btn.setColour(juce::TextButton::buttonColourId, tempolink::juceapp::style::CardBackground());
    btn.setColour(juce::TextButton::textColourOffId, tempolink::juceapp::style::TextPrimary());
  };
  update_btn(audio_settings_button_);
  update_btn(bpm_minus_button_);
  update_btn(bpm_plus_button_);

  auto update_slider = [](juce::Slider& s) {
    s.setColour(juce::Slider::thumbColourId, tempolink::juceapp::style::AccentCyan());
    s.setColour(juce::Slider::trackColourId, tempolink::juceapp::style::PanelBorder());
    s.setColour(juce::Slider::textBoxTextColourId, tempolink::juceapp::style::TextPrimary());
    s.setColour(juce::Slider::textBoxBackgroundColourId, tempolink::juceapp::style::CardBackground());
    s.setColour(juce::Slider::textBoxOutlineColourId, tempolink::juceapp::style::PanelBorder());
  };

  auto update_combo = [](juce::ComboBox& c) {
    c.setColour(juce::ComboBox::backgroundColourId, tempolink::juceapp::style::PanelBackground());
    c.setColour(juce::ComboBox::textColourId, tempolink::juceapp::style::TextPrimary());
    c.setColour(juce::ComboBox::outlineColourId, tempolink::juceapp::style::PanelBorder());
    c.setColour(juce::ComboBox::arrowColourId, tempolink::juceapp::style::TextSecondary());
  };
  update_combo(input_combo_);
  update_combo(output_combo_);
  update_combo(metronome_tone_combo_);

  repaint();
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

void MyInputPanel::setInputLevel(float level) {
  level_meter_.setLevel(std::clamp(level, 0.0F, 1.0F));
}

void MyInputPanel::setInputGain(float gain) {
  suppress_callbacks_ = true;
  input_gain_slider_.setValue(std::clamp(gain, 0.0F, 2.0F),
                              juce::dontSendNotification);
  suppress_callbacks_ = false;
}

void MyInputPanel::setInputReverb(float amount) {
  suppress_callbacks_ = true;
  reverb_slider_.setValue(std::clamp(amount, 0.0F, 1.0F),
                          juce::dontSendNotification);
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

void MyInputPanel::setMetronomeTone(int tone) {
  suppress_callbacks_ = true;
  metronome_tone_combo_.setSelectedId(juce::jlimit(1, 3, tone + 1),
                                      juce::dontSendNotification);
  suppress_callbacks_ = false;
}

void MyInputPanel::setOnMuteChanged(std::function<void(bool)> on_mute_changed) {
  on_mute_changed_ = std::move(on_mute_changed);
}

void MyInputPanel::setOnInputGainChanged(
    std::function<void(float)> on_input_gain_changed) {
  on_input_gain_changed_ = std::move(on_input_gain_changed);
}

void MyInputPanel::setOnInputReverbChanged(
    std::function<void(float)> on_input_reverb_changed) {
  on_input_reverb_changed_ = std::move(on_input_reverb_changed);
}

void MyInputPanel::setOnMetronomeChanged(
    std::function<void(bool)> on_metronome_changed) {
  on_metronome_changed_ = std::move(on_metronome_changed);
}

void MyInputPanel::setOnBpmChanged(std::function<void(int)> on_bpm_changed) {
  on_bpm_changed_ = std::move(on_bpm_changed);
}

void MyInputPanel::setOnMetronomeToneChanged(
    std::function<void(int)> on_metronome_tone_changed) {
  on_metronome_tone_changed_ = std::move(on_metronome_tone_changed);
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
  const int h = area.getHeight();
  const int label_h = juce::jlimit(18, 24, static_cast<int>(h * 0.045F));
  const int combo_h = juce::jlimit(24, 34, static_cast<int>(h * 0.065F));
  const int meter_h = juce::jlimit(10, 16, static_cast<int>(h * 0.03F));
  const int slider_h = juce::jlimit(26, 36, static_cast<int>(h * 0.07F));
  const int toggle_h = juce::jlimit(24, 34, static_cast<int>(h * 0.07F));
  const int gap_sm = juce::jlimit(2, 6, static_cast<int>(h * 0.012F));
  const int gap_md = juce::jlimit(4, 10, static_cast<int>(h * 0.018F));
  input_label_.setBounds(area.removeFromTop(label_h));
  input_combo_.setBounds(area.removeFromTop(combo_h));
  area.removeFromTop(gap_sm);
  output_label_.setBounds(area.removeFromTop(label_h));
  output_combo_.setBounds(area.removeFromTop(combo_h));
  area.removeFromTop(gap_md);
  input_level_label_.setBounds(area.removeFromTop(label_h));
  level_meter_.setBounds(area.removeFromTop(meter_h));
  area.removeFromTop(gap_sm);
  input_gain_label_.setBounds(area.removeFromTop(label_h));
  input_gain_slider_.setBounds(area.removeFromTop(slider_h));
  area.removeFromTop(gap_md);
  reverb_label_.setBounds(area.removeFromTop(label_h));
  reverb_slider_.setBounds(area.removeFromTop(slider_h));
  area.removeFromTop(gap_md);
  mute_toggle_.setBounds(area.removeFromTop(toggle_h));
  area.removeFromTop(gap_sm);
  metronome_toggle_.setBounds(area.removeFromTop(toggle_h));
  area.removeFromTop(gap_md);
  metronome_tone_label_.setBounds(area.removeFromTop(label_h));
  metronome_tone_combo_.setBounds(area.removeFromTop(combo_h));
  area.removeFromTop(gap_md);
  bpm_label_.setBounds(area.removeFromTop(label_h));
  auto bpm_row = area.removeFromTop(slider_h);
  const int button_width = juce::jlimit(28, 44, static_cast<int>(bpm_row.getHeight() * 1.1F));
  bpm_slider_.setBounds(bpm_row.removeFromLeft(
      juce::jmax(40, bpm_row.getWidth() - (button_width * 2 + 8))));
  bpm_row.removeFromLeft(4);
  bpm_minus_button_.setBounds(bpm_row.removeFromLeft(button_width));
  bpm_row.removeFromLeft(4);
  bpm_plus_button_.setBounds(bpm_row.removeFromLeft(button_width));
  area.removeFromTop(gap_md);
  const int settings_h = juce::jlimit(24, 32, static_cast<int>(h * 0.065F));
  const int settings_w = juce::jlimit(110, 180, static_cast<int>(area.getWidth() * 0.55F));
  audio_settings_button_.setBounds(area.removeFromTop(settings_h).removeFromLeft(settings_w));
}
