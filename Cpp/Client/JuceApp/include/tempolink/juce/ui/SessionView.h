#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "tempolink/juce/ui/common/PanelCard.h"
#include "tempolink/juce/ui/ParticipantStripComponent.h"
#include "tempolink/juce/ui/session/MainOutPanel.h"
#include "tempolink/juce/ui/session/MyInputPanel.h"
#include "tempolink/juce/ui/session/TopStatusBar.h"

class SessionView final : public juce::Component {
 public:
  SessionView();

  void setRoomTitle(const juce::String& title);
  void setConnectionState(bool connected);
  void setConnectionMode(ConnectionBadgeState state);
  void setStatusText(const juce::String& status_text);
  void setParticipants(const std::vector<ParticipantSummary>& participants);
  void updateParticipantLevels(const std::vector<ParticipantSummary>& participants);

  void setInputDevices(const std::vector<std::string>& devices,
                       const std::string& selected_device);
  void setOutputDevices(const std::vector<std::string>& devices,
                        const std::string& selected_device);

  void setMasterVolume(float volume);
  void setMetronomeBpm(int bpm);
  void setMute(bool muted);
  void setMetronomeEnabled(bool enabled);

  void setOnBack(std::function<void()> on_back);
  void setOnDisconnect(std::function<void()> on_disconnect);
  void setOnMuteChanged(std::function<void(bool)> on_mute_changed);
  void setOnMetronomeChanged(std::function<void(bool)> on_metronome_changed);
  void setOnVolumeChanged(std::function<void(float)> on_volume_changed);
  void setOnBpmChanged(std::function<void(int)> on_bpm_changed);
  void setOnInputDeviceChanged(std::function<void(std::string)> on_input_changed);
  void setOnOutputDeviceChanged(std::function<void(std::string)> on_output_changed);
  void setOnParticipantAudioSettings(std::function<void(std::string)> on_audio_settings);
  void setOnParticipantReconnect(std::function<void(std::string)> on_reconnect);
  void setOnOpenAudioSettings(std::function<void()> on_open_audio_settings);

  void resized() override;
  void paint(juce::Graphics& g) override;

 private:
  void rebuildParticipantRows();
  void layoutParticipantRows();

  std::vector<ParticipantSummary> participants_;
  std::vector<std::unique_ptr<ParticipantStripComponent>> participant_rows_;
  bool suppress_callbacks_ = false;

  std::function<void()> on_back_;
  std::function<void()> on_disconnect_;
  std::function<void(bool)> on_mute_changed_;
  std::function<void(bool)> on_metronome_changed_;
  std::function<void(float)> on_volume_changed_;
  std::function<void(int)> on_bpm_changed_;
  std::function<void(std::string)> on_input_changed_;
  std::function<void(std::string)> on_output_changed_;
  std::function<void(std::string)> on_participant_audio_settings_;
  std::function<void(std::string)> on_participant_reconnect_;
  std::function<void()> on_open_audio_settings_;

  TopStatusBar top_status_bar_;
  MyInputPanel my_input_panel_;
  MainOutPanel main_out_panel_;

  PanelCard participant_list_card_;
  juce::Viewport participants_viewport_;
  juce::Component participants_container_;
};
