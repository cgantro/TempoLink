#include "tempolink/juce/ui/screens/SessionView.h"

#include <algorithm>
#include <utility>

#include "tempolink/juce/style/UiStyle.h"

SessionView::SessionView() {
  addAndMakeVisible(top_status_bar_);
  addAndMakeVisible(chat_toggle_button_);
  addAndMakeVisible(my_input_panel_);
  addAndMakeVisible(main_out_panel_);

  participant_list_card_.setTitle("Participants");
  addAndMakeVisible(participant_list_card_);

  chat_card_.setTitle("Room Chat");
  addAndMakeVisible(chat_card_);
  chat_card_.setVisible(chat_open_);

  participants_viewport_.setViewedComponent(&participants_container_, false);
  participants_viewport_.setScrollBarsShown(true, false);
  participant_list_card_.setContent(participants_viewport_);

  my_input_panel_.setOnMuteChanged([this](bool muted) {
    if (!suppress_callbacks_ && on_mute_changed_) {
      on_mute_changed_(muted);
    }
  });
  my_input_panel_.setOnInputGainChanged([this](float gain) {
    if (!suppress_callbacks_ && on_input_gain_changed_) {
      on_input_gain_changed_(gain);
    }
  });
  my_input_panel_.setOnInputReverbChanged([this](float amount) {
    if (!suppress_callbacks_ && on_input_reverb_changed_) {
      on_input_reverb_changed_(amount);
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
  my_input_panel_.setOnMetronomeToneChanged([this](int tone) {
    if (!suppress_callbacks_ && on_metronome_tone_changed_) {
      on_metronome_tone_changed_(tone);
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
  main_out_panel_.setOnAudioFileToggle([this](bool active) {
    if (!suppress_callbacks_ && on_audio_file_toggle_) {
      on_audio_file_toggle_(active);
    }
  });
  main_out_panel_.setOnAudioFileSeekChanged([this](float position) {
    if (!suppress_callbacks_ && on_audio_seek_changed_) {
      on_audio_seek_changed_(position);
    }
  });
  main_out_panel_.setOnAudioFileLoopChanged([this](bool enabled) {
    if (!suppress_callbacks_ && on_audio_loop_changed_) {
      on_audio_loop_changed_(enabled);
    }
  });
  main_out_panel_.setOnRecordToggle([this](bool recording) {
    if (!suppress_callbacks_ && on_record_toggle_) {
      on_record_toggle_(recording);
    }
  });
  main_out_panel_.setOnDisconnect([this] {
    if (on_disconnect_) {
      on_disconnect_();
    }
  });

  chat_toggle_button_.setClickingTogglesState(true);
  chat_toggle_button_.setToggleState(true, juce::dontSendNotification);
  chat_toggle_button_.onClick = [this] {
    chat_open_ = chat_toggle_button_.getToggleState();
    chat_card_.setVisible(chat_open_);
    updateChatToggleButtonText();
    resized();
  };
  updateChatToggleButtonText();
  
  updateTheme();
}
void SessionView::updateTheme() {
  participant_list_card_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());
  chat_toggle_button_.setColour(juce::TextButton::buttonColourId,
                                tempolink::juceapp::style::CardBackground());
  chat_toggle_button_.setColour(juce::TextButton::buttonOnColourId,
                                tempolink::juceapp::style::AccentCyan().withAlpha(0.28F));
  chat_toggle_button_.setColour(juce::TextButton::textColourOffId,
                                tempolink::juceapp::style::TextPrimary());
  chat_toggle_button_.setColour(juce::TextButton::textColourOnId,
                                tempolink::juceapp::style::TextPrimary());
  
  // Note: child panels (TopStatusBar, etc) are their own listeners and handle themselves.
  // But we can force them to update if needed, though they already do via the global manager.
  
  repaint();
}

void SessionView::setRoomTitle(const std::string& title) {
  top_status_bar_.setRoomTitle(juce::String(title));
}

void SessionView::setConnectionState(bool connected) {
  setConnectionMode(connected ? ConnectionBadgeState::P2P
                              : ConnectionBadgeState::Reconnecting);
}

void SessionView::setConnectionMode(ConnectionBadgeState state) {
  top_status_bar_.setConnectionMode(state);
}

void SessionView::setStatusText(const std::string& status_text) {
  top_status_bar_.setStatusText(juce::String(status_text));
}

void SessionView::setSignalingClient(SignalingClient& client) {
  chat_panel_ = std::make_unique<tempolink::juceapp::ui::ChatPanel>(client);
  chat_card_.setContent(*chat_panel_);
  resized();
}

void SessionView::addChatMessage(const std::string& user_id, const std::string& text, bool is_local) {
  if (chat_panel_) {
    tempolink::juceapp::ui::ChatPanel::Message msg;
    msg.user_id = user_id;
    msg.text = juce::String(text);
    msg.is_local = is_local;
    msg.timestamp = juce::Time::getCurrentTime().toString(true, false);
    chat_panel_->AddMessage(msg);
  }
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
#if JUCE_DEBUG
    const auto& p = participants.at(i);
    participant_rows_.at(i)->setRuntimeMetrics(
        p.level, p.latency_ms, p.packet_loss_percent, p.connection_state);
#else
    const auto& p = participants[i];
    participant_rows_[i]->setRuntimeMetrics(
        p.level, p.latency_ms, p.packet_loss_percent, p.connection_state);
#endif
  }
}

void SessionView::setParticipantMonitorVolume(const std::string& user_id, float volume) {
  for (std::size_t i = 0; i < participants_.size(); ++i) {
    if (participants_[i].user_id != user_id) {
      continue;
    }
    participants_[i].monitor_volume = volume;
    if (i < participant_rows_.size()) {
      participant_rows_[i]->setMonitorMix(participants_[i].monitor_volume,
                                          participants_[i].monitor_pan);
    }
    break;
  }
}

void SessionView::setParticipantMonitorPan(const std::string& user_id, float pan) {
  for (std::size_t i = 0; i < participants_.size(); ++i) {
    if (participants_[i].user_id != user_id) {
      continue;
    }
    participants_[i].monitor_pan = pan;
    if (i < participant_rows_.size()) {
      participant_rows_[i]->setMonitorMix(participants_[i].monitor_volume,
                                          participants_[i].monitor_pan);
    }
    break;
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

void SessionView::setInputLevel(float level) {
  my_input_panel_.setInputLevel(level);
}

void SessionView::setInputGain(float gain) {
  my_input_panel_.setInputGain(gain);
}

void SessionView::setInputReverb(float amount) {
  my_input_panel_.setInputReverb(amount);
}

void SessionView::setRecording(bool recording) {
  main_out_panel_.setRecording(recording);
}

void SessionView::setAudioFileActive(bool active) {
  main_out_panel_.setAudioFileActive(active);
}

void SessionView::setAudioFilePlaybackPosition(float normalized_position) {
  main_out_panel_.setAudioFilePlaybackPosition(normalized_position);
}

void SessionView::setAudioFileLoopEnabled(bool enabled) {
  main_out_panel_.setAudioFileLoopEnabled(enabled);
}

void SessionView::setMetronomeBpm(int bpm) {
  my_input_panel_.setMetronomeBpm(bpm);
}

void SessionView::setMetronomeTone(int tone) {
  my_input_panel_.setMetronomeTone(tone);
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

void SessionView::setOnInputGainChanged(
    std::function<void(float)> on_input_gain_changed) {
  on_input_gain_changed_ = std::move(on_input_gain_changed);
}

void SessionView::setOnInputReverbChanged(
    std::function<void(float)> on_input_reverb_changed) {
  on_input_reverb_changed_ = std::move(on_input_reverb_changed);
}

void SessionView::setOnMetronomeChanged(
    std::function<void(bool)> on_metronome_changed) {
  on_metronome_changed_ = std::move(on_metronome_changed);
}

void SessionView::setOnVolumeChanged(std::function<void(float)> on_volume_changed) {
  on_volume_changed_ = std::move(on_volume_changed);
}

void SessionView::setOnAudioFileToggle(
    std::function<void(bool)> on_audio_file_toggle) {
  on_audio_file_toggle_ = std::move(on_audio_file_toggle);
}

void SessionView::setOnAudioFileSeekChanged(
    std::function<void(float)> on_audio_seek_changed) {
  on_audio_seek_changed_ = std::move(on_audio_seek_changed);
}

void SessionView::setOnAudioFileLoopChanged(
    std::function<void(bool)> on_audio_loop_changed) {
  on_audio_loop_changed_ = std::move(on_audio_loop_changed);
}

void SessionView::setOnRecordToggle(std::function<void(bool)> on_record_toggle) {
  on_record_toggle_ = std::move(on_record_toggle);
}

void SessionView::setOnBpmChanged(std::function<void(int)> on_bpm_changed) {
  on_bpm_changed_ = std::move(on_bpm_changed);
}

void SessionView::setOnMetronomeToneChanged(
    std::function<void(int)> on_metronome_tone_changed) {
  on_metronome_tone_changed_ = std::move(on_metronome_tone_changed);
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

void SessionView::setOnParticipantMonitorVolumeChanged(
    std::function<void(std::string, float)> on_volume_changed) {
  on_participant_monitor_volume_changed_ = std::move(on_volume_changed);
  for (auto& row : participant_rows_) {
    row->setOnMonitorVolumeChanged(on_participant_monitor_volume_changed_);
  }
}

void SessionView::setOnParticipantMonitorPanChanged(
    std::function<void(std::string, float)> on_pan_changed) {
  on_participant_monitor_pan_changed_ = std::move(on_pan_changed);
  for (auto& row : participant_rows_) {
    row->setOnMonitorPanChanged(on_participant_monitor_pan_changed_);
  }
}

void SessionView::setOnOpenAudioSettings(std::function<void()> on_open_audio_settings) {
  on_open_audio_settings_ = std::move(on_open_audio_settings);
}

void SessionView::resized() {
  auto area = getLocalBounds().reduced(tempolink::juceapp::style::kSessionPadding);

  const int total_h = area.getHeight();
  const int total_w = area.getWidth();
  const int top_h = juce::jlimit(76, 110, static_cast<int>(total_h * 0.135F));
  const int section_gap = juce::jlimit(6, 14, static_cast<int>(total_h * 0.012F));
  top_status_bar_.setBounds(area.removeFromTop(top_h));
  const int toggle_w = juce::jlimit(96, 140, static_cast<int>(total_w * 0.11F));
  const int toggle_h = juce::jlimit(28, 36, static_cast<int>(top_h * 0.38F));
  chat_toggle_button_.setBounds(
      top_status_bar_.getRight() - toggle_w - 14,
      top_status_bar_.getY() + juce::jmax(8, (top_h - toggle_h) / 2),
      toggle_w, toggle_h);
  area.removeFromTop(section_gap);

  const int left_w = juce::jlimit(260, 420, static_cast<int>(total_w * 0.30F));
  auto left = area.removeFromLeft(left_w);
  area.removeFromLeft(section_gap);
  auto center = area;
  juce::Rectangle<int> chat_area;
  if (chat_open_) {
    const int chat_w = juce::jlimit(220, 420, static_cast<int>(center.getWidth() * 0.30F));
    chat_area = center.removeFromRight(chat_w);
    center.removeFromRight(section_gap);
  }

  // Responsive split for input/main-out panel.
  const int min_main_out_height = juce::jlimit(84, 130, static_cast<int>(left.getHeight() * 0.22F));
  const int desired_input_height = static_cast<int>(left.getHeight() * 0.74F);
  const int max_input_height = juce::jmax(220, left.getHeight() - min_main_out_height);
  const int input_height = juce::jlimit(220, max_input_height, desired_input_height);
  my_input_panel_.setBounds(left.removeFromTop(input_height));
  left.removeFromTop(section_gap);
  main_out_panel_.setBounds(left);

  participant_list_card_.setBounds(center);
  if (chat_open_) {
    chat_card_.setVisible(true);
    chat_card_.setBounds(chat_area);
  } else {
    chat_card_.setVisible(false);
    chat_card_.setBounds({});
  }

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
    row->setOnMonitorVolumeChanged(on_participant_monitor_volume_changed_);
    row->setOnMonitorPanChanged(on_participant_monitor_pan_changed_);
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
  const int row_height = tempolink::juceapp::style::kParticipantRowHeight + 82;

  int y = padding;
  for (auto& row : participant_rows_) {
    row->setBounds(padding, y, width - padding * 2 - 16, row_height);
    y += row_height + padding;
  }
  participants_container_.setSize(width, y + padding);
}

void SessionView::updateChatToggleButtonText() {
  chat_toggle_button_.setButtonText(chat_open_ ? "Hide Chat" : "Show Chat");
}
