#include "tempolink/juce/ui/SessionView.h"

#include <algorithm>
#include <utility>

#include "tempolink/juce/style/UiStyle.h"

SessionView::SessionView() {
  addAndMakeVisible(top_status_bar_);
  addAndMakeVisible(my_input_panel_);
  addAndMakeVisible(main_out_panel_);

  participant_list_card_.setTitle("Participants");
  addAndMakeVisible(participant_list_card_);

  participants_viewport_.setViewedComponent(&participants_container_, false);
  participants_viewport_.setScrollBarsShown(true, false);
  participant_list_card_.setContent(participants_viewport_);

  my_input_panel_.setOnMuteChanged([this](bool muted) {
    if (!suppress_callbacks_ && on_mute_changed_) {
      on_mute_changed_(muted);
    }
  });
  my_input_panel_.setOnMetronomeChanged([this](bool enabled) {
    if (!suppress_callbacks_ && on_metronome_changed_) {
      on_metronome_changed_(enabled);
    }
  });
  my_input_panel_.setOnBpmChanged([this](int bpm) {
    if (!suppress_callbacks_ && on_bpm_changed_) {
      on_bpm_changed_(bpm);
    }
  });
  my_input_panel_.setOnInputDeviceChanged([this](std::string device_id) {
    if (!suppress_callbacks_ && on_input_changed_) {
      on_input_changed_(std::move(device_id));
    }
  });
  my_input_panel_.setOnOutputDeviceChanged([this](std::string device_id) {
    if (!suppress_callbacks_ && on_output_changed_) {
      on_output_changed_(std::move(device_id));
    }
  });
  my_input_panel_.setOnOpenAudioSettings([this] {
    if (!suppress_callbacks_ && on_open_audio_settings_) {
      on_open_audio_settings_();
    }
  });

  main_out_panel_.setOnVolumeChanged([this](float volume) {
    if (!suppress_callbacks_ && on_volume_changed_) {
      on_volume_changed_(volume);
    }
  });
  main_out_panel_.setOnDisconnect([this] {
    if (on_disconnect_) {
      on_disconnect_();
    }
  });
}

void SessionView::setRoomTitle(const juce::String& title) {
  top_status_bar_.setRoomTitle(title);
}

void SessionView::setConnectionState(bool connected) {
  setConnectionMode(connected ? ConnectionBadgeState::Relay
                              : ConnectionBadgeState::Reconnecting);
}

void SessionView::setConnectionMode(ConnectionBadgeState state) {
  top_status_bar_.setConnectionMode(state);
}

void SessionView::setStatusText(const juce::String& status_text) {
  top_status_bar_.setStatusText(status_text);
}

void SessionView::setParticipants(
    const std::vector<ParticipantSummary>& participants) {
  participants_ = participants;
  rebuildParticipantRows();
  resized();
}

void SessionView::updateParticipantLevels(
    const std::vector<ParticipantSummary>& participants) {
  const std::size_t count = std::min(participants.size(), participant_rows_.size());
  for (std::size_t i = 0; i < count; ++i) {
    participant_rows_[i]->setParticipant(participants[i]);
  }
}

void SessionView::setInputDevices(const std::vector<std::string>& devices,
                                  const std::string& selected_device) {
  my_input_panel_.setInputDevices(devices, selected_device);
}

void SessionView::setOutputDevices(const std::vector<std::string>& devices,
                                   const std::string& selected_device) {
  my_input_panel_.setOutputDevices(devices, selected_device);
}

void SessionView::setMasterVolume(float volume) {
  main_out_panel_.setMasterVolume(volume);
}

void SessionView::setMetronomeBpm(int bpm) {
  my_input_panel_.setMetronomeBpm(bpm);
}

void SessionView::setMute(bool muted) {
  my_input_panel_.setMute(muted);
}

void SessionView::setMetronomeEnabled(bool enabled) {
  my_input_panel_.setMetronomeEnabled(enabled);
}

void SessionView::setOnBack(std::function<void()> on_back) {
  on_back_ = std::move(on_back);
  top_status_bar_.setOnBack([this] {
    if (on_back_) {
      on_back_();
    }
  });
}

void SessionView::setOnDisconnect(std::function<void()> on_disconnect) {
  on_disconnect_ = std::move(on_disconnect);
}

void SessionView::setOnMuteChanged(std::function<void(bool)> on_mute_changed) {
  on_mute_changed_ = std::move(on_mute_changed);
}

void SessionView::setOnMetronomeChanged(
    std::function<void(bool)> on_metronome_changed) {
  on_metronome_changed_ = std::move(on_metronome_changed);
}

void SessionView::setOnVolumeChanged(std::function<void(float)> on_volume_changed) {
  on_volume_changed_ = std::move(on_volume_changed);
}

void SessionView::setOnBpmChanged(std::function<void(int)> on_bpm_changed) {
  on_bpm_changed_ = std::move(on_bpm_changed);
}

void SessionView::setOnInputDeviceChanged(
    std::function<void(std::string)> on_input_changed) {
  on_input_changed_ = std::move(on_input_changed);
}

void SessionView::setOnOutputDeviceChanged(
    std::function<void(std::string)> on_output_changed) {
  on_output_changed_ = std::move(on_output_changed);
}

void SessionView::setOnParticipantAudioSettings(
    std::function<void(std::string)> on_audio_settings) {
  on_participant_audio_settings_ = std::move(on_audio_settings);
  for (auto& row : participant_rows_) {
    row->setOnAudioSettingsClicked(on_participant_audio_settings_);
  }
}

void SessionView::setOnParticipantReconnect(
    std::function<void(std::string)> on_reconnect) {
  on_participant_reconnect_ = std::move(on_reconnect);
  for (auto& row : participant_rows_) {
    row->setOnReconnectClicked(on_participant_reconnect_);
  }
}

void SessionView::setOnOpenAudioSettings(std::function<void()> on_open_audio_settings) {
  on_open_audio_settings_ = std::move(on_open_audio_settings);
}

void SessionView::resized() {
  auto area = getLocalBounds().reduced(tempolink::juceapp::style::kSessionPadding);

  top_status_bar_.setBounds(area.removeFromTop(92));
  area.removeFromTop(10);

  auto left = area.removeFromLeft(tempolink::juceapp::style::kSessionLeftPanelWidth);
  auto center = area;

  my_input_panel_.setBounds(left.removeFromTop(320));
  left.removeFromTop(8);
  main_out_panel_.setBounds(left);

  participant_list_card_.setBounds(center);

  layoutParticipantRows();
}

void SessionView::paint(juce::Graphics& g) {
  g.fillAll(tempolink::juceapp::style::SessionBackground());
}

void SessionView::rebuildParticipantRows() {
  participant_rows_.clear();
  participants_container_.removeAllChildren();

  for (const auto& participant : participants_) {
    auto row = std::make_unique<ParticipantStripComponent>();
    row->setParticipant(participant);
    row->setOnAudioSettingsClicked(on_participant_audio_settings_);
    row->setOnReconnectClicked(on_participant_reconnect_);
    participants_container_.addAndMakeVisible(*row);
    participant_rows_.push_back(std::move(row));
  }
}

void SessionView::layoutParticipantRows() {
  if (participants_viewport_.getWidth() <= 0) {
    return;
  }

  const int width = participants_viewport_.getWidth();
  const int padding = tempolink::juceapp::style::kParticipantRowPadding;
  const int row_height = tempolink::juceapp::style::kParticipantRowHeight + 34;

  int y = padding;
  for (auto& row : participant_rows_) {
    row->setBounds(padding, y, width - padding * 2 - 16, row_height);
    y += row_height + padding;
  }
  participants_container_.setSize(width, y + padding);
}
