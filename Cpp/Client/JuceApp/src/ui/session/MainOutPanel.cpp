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
  body_.addAndMakeVisible(master_label_);

  master_slider_.setSliderStyle(juce::Slider::LinearHorizontal);
  master_slider_.setRange(0.0, 1.0, 0.01);
  master_slider_.setValue(1.0);
  body_.addAndMakeVisible(master_slider_);

  body_.addAndMakeVisible(disconnect_button_);

  master_slider_.onValueChange = [this] {
    if (!suppress_callbacks_ && on_volume_changed_) {
      on_volume_changed_(static_cast<float>(master_slider_.getValue()));
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

void MainOutPanel::setOnVolumeChanged(std::function<void(float)> on_volume_changed) {
  on_volume_changed_ = std::move(on_volume_changed);
}

void MainOutPanel::setOnDisconnect(std::function<void()> on_disconnect) {
  on_disconnect_ = std::move(on_disconnect);
}

void MainOutPanel::resized() {
  card_.setBounds(getLocalBounds());
  auto area = body_.getLocalBounds();
  master_label_.setBounds(area.removeFromTop(20));
  master_slider_.setBounds(area.removeFromTop(24));
  area.removeFromTop(10);
  disconnect_button_.setBounds(area.removeFromTop(28).removeFromLeft(140));
}
