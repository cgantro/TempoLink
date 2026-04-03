#pragma once

#include <string>
#include <vector>
#include <functional>

#include "tempolink/juce/ui/models/UiModels.h"

class ISessionView {
 public:
  virtual ~ISessionView() = default;

  virtual void setRoomTitle(const std::string& title) = 0;
  virtual void setConnectionState(bool connected) = 0;
  virtual void setConnectionMode(ConnectionBadgeState state) = 0;
  virtual void setStatusText(const std::string& status_text) = 0;
  virtual void setParticipants(const std::vector<ParticipantSummary>& participants) = 0;
  virtual void updateParticipantLevels(const std::vector<ParticipantSummary>& participants) = 0;
  virtual void setParticipantMonitorVolume(const std::string& user_id, float volume) = 0;
  virtual void setParticipantMonitorPan(const std::string& user_id, float pan) = 0;

  virtual void setInputDevices(const std::vector<std::string>& devices, const std::string& selected_device) = 0;
  virtual void setOutputDevices(const std::vector<std::string>& devices, const std::string& selected_device) = 0;

  virtual void setMasterVolume(float volume) = 0;
  virtual void setInputLevel(float level) = 0;
  virtual void setInputGain(float gain) = 0;
  virtual void setInputReverb(float amount) = 0;
  virtual void setRecording(bool recording) = 0;
  virtual void setAudioFileActive(bool active) = 0;
  virtual void setAudioFilePlaybackPosition(float normalized_position) = 0;
  virtual void setAudioFileLoopEnabled(bool enabled) = 0;
  virtual void setMetronomeBpm(int bpm) = 0;
  virtual void setMetronomeTone(int tone) = 0;
  virtual void setMute(bool muted) = 0;
  virtual void setMetronomeEnabled(bool enabled) = 0;

  virtual void setOnBack(std::function<void()> on_back) = 0;
  virtual void setOnDisconnect(std::function<void()> on_disconnect) = 0;
  virtual void setOnMuteChanged(std::function<void(bool)> on_mute_changed) = 0;
  virtual void setOnInputGainChanged(std::function<void(float)> on_input_gain_changed) = 0;
  virtual void setOnInputReverbChanged(std::function<void(float)> on_input_reverb_changed) = 0;
  virtual void setOnMetronomeChanged(std::function<void(bool)> on_metronome_changed) = 0;
  virtual void setOnVolumeChanged(std::function<void(float)> on_volume_changed) = 0;
  virtual void setOnAudioFileToggle(std::function<void(bool)> on_audio_file_toggle) = 0;
  virtual void setOnAudioFileSeekChanged(std::function<void(float)> on_audio_seek_changed) = 0;
  virtual void setOnAudioFileLoopChanged(std::function<void(bool)> on_audio_loop_changed) = 0;
  virtual void setOnRecordToggle(std::function<void(bool)> on_record_toggle) = 0;
  virtual void setOnBpmChanged(std::function<void(int)> on_bpm_changed) = 0;
  virtual void setOnMetronomeToneChanged(
      std::function<void(int)> on_metronome_tone_changed) = 0;
  virtual void setOnInputDeviceChanged(std::function<void(std::string)> on_input_changed) = 0;
  virtual void setOnOutputDeviceChanged(std::function<void(std::string)> on_output_changed) = 0;
  virtual void setOnParticipantAudioSettings(std::function<void(std::string)> on_audio_settings) = 0;
  virtual void setOnParticipantReconnect(std::function<void(std::string)> on_reconnect) = 0;
  virtual void setOnParticipantMonitorVolumeChanged(std::function<void(std::string, float)> on_volume_changed) = 0;
  virtual void setOnParticipantMonitorPanChanged(std::function<void(std::string, float)> on_pan_changed) = 0;
  virtual void setOnOpenAudioSettings(std::function<void()> on_open_audio_settings) = 0;

  virtual void addChatMessage(const std::string& user_id, const std::string& text, bool is_local = false) = 0;
};
