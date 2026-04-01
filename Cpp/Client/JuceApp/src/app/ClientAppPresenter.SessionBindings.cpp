#include "tempolink/juce/app/ClientAppPresenter.h"

#include "tempolink/juce/app/SessionBindings.h"
#include "tempolink/juce/app/SessionModelSupport.h"

void ClientAppPresenter::wireSessionUiEvents() {
  using ScreenMode = tempolink::juceapp::app::ScreenMode;

  tempolink::juceapp::app::SessionBindingsCallbacks session_callbacks;
  session_callbacks.on_back = [this] {
    session_recording_ = false;
    session_audio_file_active_ = false;
    leaveRoom();
    setLobbyStatusText("Returned to lobby.");
    screen_navigator_.SwitchTo(ScreenMode::Lobby);
  };
  session_callbacks.on_disconnect = [this] {
    session_recording_ = false;
    session_audio_file_active_ = false;
    leaveRoom();
    setLobbyStatusText("Disconnected from room.");
    screen_navigator_.SwitchTo(ScreenMode::Lobby);
  };
  session_callbacks.on_mute_changed = [this](bool muted) {
    if (!session_lifecycle_controller_.session_active()) {
      return;
    }
    session_.SetMuted(muted);
    session_presence_controller_.updateSelfMuted(muted);
  };
  session_callbacks.on_input_gain_changed = [this](float gain) {
    if (session_lifecycle_controller_.session_active()) {
      session_.SetInputGain(gain);
      session_view_.setStatusText("Input gain: " +
                                  juce::String(gain, 2) + "x");
    }
  };
  session_callbacks.on_input_reverb_changed = [this](float amount) {
    if (session_lifecycle_controller_.session_active()) {
      session_.SetInputReverb(amount);
      session_view_.setStatusText("Reverb: " + juce::String(amount, 2));
    }
  };
  session_callbacks.on_metronome_changed = [this](bool enabled) {
    if (enabled && session_audio_file_active_) {
      session_view_.setMetronomeEnabled(false);
      session_view_.setStatusText(
          "Audio file playback and metronome cannot run together.");
      return;
    }
    if (session_lifecycle_controller_.session_active()) {
      session_.SetMetronomeEnabled(enabled);
    }
  };
  session_callbacks.on_volume_changed = [this](float volume) {
    if (session_lifecycle_controller_.session_active()) {
      session_.SetVolume(volume);
    }
  };
  session_callbacks.on_audio_file_toggle = [this](bool active) {
    session_audio_file_active_ = active;
    if (active && session_.IsMetronomeEnabled()) {
      session_.SetMetronomeEnabled(false);
      session_view_.setMetronomeEnabled(false);
    }
    if (active && session_recording_) {
      session_recording_ = false;
      session_view_.setRecording(false);
    }
    session_view_.setStatusText(active
                                    ? "Audio file player opened (shared playback stub)."
                                    : "Audio file player stopped.");
  };
  session_callbacks.on_record_toggle = [this](bool recording) {
    if (recording && session_audio_file_active_) {
      session_audio_file_active_ = false;
      session_view_.setAudioFileActive(false);
    }
    session_recording_ = recording;
    session_view_.setStatusText(recording ? "Recording started (stub)."
                                          : "Recording stopped.");
  };
  session_callbacks.on_bpm_changed = [this](int bpm) {
    if (session_lifecycle_controller_.session_active()) {
      session_.SetMetronomeBpm(bpm);
    }
  };
  session_callbacks.on_input_device_changed = [this](std::string device_id) {
    if (session_lifecycle_controller_.session_active()) {
      session_.SetInputDevice(device_id);
    }
  };
  session_callbacks.on_output_device_changed = [this](std::string device_id) {
    if (session_lifecycle_controller_.session_active()) {
      session_.SetOutputDevice(device_id);
    }
  };
  session_callbacks.on_open_audio_settings = [this] {
    session_view_.setStatusText("Audio settings dialog will be connected next.");
  };
  session_callbacks.on_participant_audio_settings = [this](std::string user_id) {
    if (user_id.empty()) {
      return;
    }
    session_view_.setStatusText("Participant audio settings: " + juce::String(user_id));
  };
  session_callbacks.on_participant_reconnect = [this](std::string user_id) {
    if (session_presence_controller_.sendReconnectProbe(
            user_id, session_lifecycle_controller_.session_active())) {
      session_view_.setStatusText("Peer reconnect probe sent: " + juce::String(user_id));
    } else {
      session_view_.setStatusText("Peer reconnect probe failed: " + juce::String(user_id));
    }
  };
  session_callbacks.on_participant_monitor_volume_changed =
      [this](std::string user_id, float volume) {
        if (user_id.empty()) {
          return;
        }
        session_presence_controller_.updateParticipantMonitorVolume(user_id, volume);
        if (!session_lifecycle_controller_.session_active()) {
          return;
        }
        const auto participant_id = tempolink::juceapp::app::HashToU32(user_id);
        session_.SetPeerMonitorVolume(participant_id, volume);
      };
  session_callbacks.on_participant_monitor_pan_changed =
      [this](std::string user_id, float pan) {
        if (user_id.empty()) {
          return;
        }
        session_presence_controller_.updateParticipantMonitorPan(user_id, pan);
        if (!session_lifecycle_controller_.session_active()) {
          return;
        }
        const auto participant_id = tempolink::juceapp::app::HashToU32(user_id);
        session_.SetPeerMonitorPan(participant_id, pan);
      };
  tempolink::juceapp::app::BindSessionView(session_view_, session_callbacks);
}
