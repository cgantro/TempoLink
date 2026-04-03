#pragma once

#include <functional>
#include <string>

#include "tempolink/juce/ui/interfaces/ISessionView.h"

namespace tempolink::juceapp::app {

struct SessionBindingsCallbacks {
  std::function<void()> on_back;
  std::function<void()> on_disconnect;
  std::function<void(bool)> on_mute_changed;
  std::function<void(float)> on_input_gain_changed;
  std::function<void(float)> on_input_reverb_changed;
  std::function<void(bool)> on_metronome_changed;
  std::function<void(float)> on_volume_changed;
  std::function<void(bool)> on_audio_file_toggle;
  std::function<void(float)> on_audio_file_seek_changed;
  std::function<void(bool)> on_audio_file_loop_changed;
  std::function<void(bool)> on_record_toggle;
  std::function<void(int)> on_bpm_changed;
  std::function<void(int)> on_metronome_tone_changed;
  std::function<void(std::string)> on_input_device_changed;
  std::function<void(std::string)> on_output_device_changed;
  std::function<void()> on_open_audio_settings;
  std::function<void(std::string)> on_participant_audio_settings;
  std::function<void(std::string)> on_participant_reconnect;
  std::function<void(std::string, float)> on_participant_monitor_volume_changed;
  std::function<void(std::string, float)> on_participant_monitor_pan_changed;
};

void BindSessionView(ISessionView& session_view,
                     const SessionBindingsCallbacks& callbacks);

}  // namespace tempolink::juceapp::app
