#include "tempolink/juce/app/SessionBindings.h"

namespace tempolink::juceapp::app {

void BindSessionView(SessionView& session_view,
                     const SessionBindingsCallbacks& callbacks) {
  session_view.setOnBack(callbacks.on_back);
  session_view.setOnDisconnect(callbacks.on_disconnect);
  session_view.setOnMuteChanged(callbacks.on_mute_changed);
  session_view.setOnMetronomeChanged(callbacks.on_metronome_changed);
  session_view.setOnVolumeChanged(callbacks.on_volume_changed);
  session_view.setOnBpmChanged(callbacks.on_bpm_changed);
  session_view.setOnInputDeviceChanged(callbacks.on_input_device_changed);
  session_view.setOnOutputDeviceChanged(callbacks.on_output_device_changed);
  session_view.setOnOpenAudioSettings(callbacks.on_open_audio_settings);
  session_view.setOnParticipantAudioSettings(
      callbacks.on_participant_audio_settings);
  session_view.setOnParticipantReconnect(callbacks.on_participant_reconnect);
}

}  // namespace tempolink::juceapp::app

