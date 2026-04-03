#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>
#include "tempolink/juce/style/ThemeableComponent.h"

#include "tempolink/juce/ui/common/PanelCard.h"
#include "tempolink/juce/ui/components/ParticipantStripComponent.h"
#include "tempolink/juce/ui/components/ChatPanel.h"
#include "tempolink/juce/ui/session/MainOutPanel.h"
#include "tempolink/juce/ui/session/MyInputPanel.h"
#include "tempolink/juce/ui/session/TopStatusBar.h"
#include "tempolink/juce/ui/interfaces/ISessionView.h"

class SessionView final : public tempolink::juceapp::style::ThemeableComponent,
                          public ISessionView {
 public:
  SessionView();
  void updateTheme() override;

  void setRoomTitle(const std::string& title) override;
  void setConnectionState(bool connected) override;
  void setConnectionMode(ConnectionBadgeState state) override;
  void setStatusText(const std::string& status_text) override;
  void setParticipants(const std::vector<ParticipantSummary>& participants) override;
  void updateParticipantLevels(const std::vector<ParticipantSummary>& participants) override;
  void setParticipantMonitorVolume(const std::string& user_id, float volume) override;
  void setParticipantMonitorPan(const std::string& user_id, float pan) override;

  void setInputDevices(const std::vector<std::string>& devices,
                       const std::string& selected_device) override;
  void setOutputDevices(const std::vector<std::string>& devices,
                        const std::string& selected_device) override;

  void setMasterVolume(float volume) override;
  void setInputLevel(float level) override;
  void setInputGain(float gain) override;
  void setInputReverb(float amount) override;
  void setRecording(bool recording) override;
  void setAudioFileActive(bool active) override;
  void setAudioFilePlaybackPosition(float normalized_position) override;
  void setAudioFileLoopEnabled(bool enabled) override;
  void setMetronomeBpm(int bpm) override;
  void setMetronomeTone(int tone) override;
  void setMute(bool muted) override;
  void setMetronomeEnabled(bool enabled) override;

  void setOnBack(std::function<void()> on_back) override;
  void setOnDisconnect(std::function<void()> on_disconnect) override;
  void setOnMuteChanged(std::function<void(bool)> on_mute_changed) override;
  void setOnInputGainChanged(std::function<void(float)> on_input_gain_changed) override;
  void setOnInputReverbChanged(std::function<void(float)> on_input_reverb_changed) override;
  void setOnMetronomeChanged(std::function<void(bool)> on_metronome_changed) override;
  void setOnVolumeChanged(std::function<void(float)> on_volume_changed) override;
  void setOnAudioFileToggle(std::function<void(bool)> on_audio_file_toggle) override;
  void setOnAudioFileSeekChanged(
      std::function<void(float)> on_audio_seek_changed) override;
  void setOnAudioFileLoopChanged(
      std::function<void(bool)> on_audio_loop_changed) override;
  void setOnRecordToggle(std::function<void(bool)> on_record_toggle) override;
  void setOnBpmChanged(std::function<void(int)> on_bpm_changed) override;
  void setOnMetronomeToneChanged(
      std::function<void(int)> on_metronome_tone_changed) override;
  void setOnInputDeviceChanged(std::function<void(std::string)> on_input_changed) override;
  void setOnOutputDeviceChanged(std::function<void(std::string)> on_output_changed) override;
  void setOnParticipantAudioSettings(std::function<void(std::string)> on_audio_settings) override;
  void setOnParticipantReconnect(std::function<void(std::string)> on_reconnect) override;
  void setOnParticipantMonitorVolumeChanged(
      std::function<void(std::string, float)> on_volume_changed) override;
  void setOnParticipantMonitorPanChanged(
      std::function<void(std::string, float)> on_pan_changed) override;
  void setOnOpenAudioSettings(std::function<void()> on_open_audio_settings) override;

  void setSignalingClient(SignalingClient& client);
  void addChatMessage(const std::string& user_id, const std::string& text, bool is_local = false) override;

  void resized() override;
  void paint(juce::Graphics& g) override;

 private:
  void rebuildParticipantRows();
  void layoutParticipantRows();
  void updateChatToggleButtonText();

  std::vector<ParticipantSummary> participants_;
  std::vector<std::unique_ptr<ParticipantStripComponent>> participant_rows_;
  bool suppress_callbacks_ = false;

  std::function<void()> on_back_;
  std::function<void()> on_disconnect_;
  std::function<void(bool)> on_mute_changed_;
  std::function<void(float)> on_input_gain_changed_;
  std::function<void(float)> on_input_reverb_changed_;
  std::function<void(bool)> on_metronome_changed_;
  std::function<void(float)> on_volume_changed_;
  std::function<void(bool)> on_audio_file_toggle_;
  std::function<void(float)> on_audio_seek_changed_;
  std::function<void(bool)> on_audio_loop_changed_;
  std::function<void(bool)> on_record_toggle_;
  std::function<void(int)> on_bpm_changed_;
  std::function<void(int)> on_metronome_tone_changed_;
  std::function<void(std::string)> on_input_changed_;
  std::function<void(std::string)> on_output_changed_;
  std::function<void(std::string)> on_participant_audio_settings_;
  std::function<void(std::string)> on_participant_reconnect_;
  std::function<void(std::string, float)> on_participant_monitor_volume_changed_;
  std::function<void(std::string, float)> on_participant_monitor_pan_changed_;
  std::function<void()> on_open_audio_settings_;

  TopStatusBar top_status_bar_;
  juce::TextButton chat_toggle_button_{"Hide Chat"};
  bool chat_open_ = true;
  MyInputPanel my_input_panel_;
  MainOutPanel main_out_panel_;

  PanelCard participant_list_card_;
  juce::Viewport participants_viewport_;
  juce::Component participants_container_;

  PanelCard chat_card_;
  std::unique_ptr<tempolink::juceapp::ui::ChatPanel> chat_panel_;
};
