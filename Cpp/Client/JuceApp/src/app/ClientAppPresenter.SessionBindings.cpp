#include "tempolink/juce/app/ClientAppPresenter.h"

#include <algorithm>

#include <juce_gui_basics/juce_gui_basics.h>

#include "tempolink/juce/app/SessionBindings.h"
#include "tempolink/juce/app/SessionModelSupport.h"
#include "tempolink/juce/bridge/AudioSessionService.h"

void ClientAppPresenter::wireSessionUiEvents() {
  using ScreenMode = tempolink::juceapp::app::ScreenMode;

  tempolink::juceapp::app::SessionBindingsCallbacks session_callbacks;
  session_callbacks.on_back = [this] {
    if (audio_session_service_) {
      audio_session_service_->StopAudioFilePlayback();
      juce::String ignored_path;
      juce::String ignored_error;
      if (audio_session_service_->IsRecording()) {
        audio_session_service_->StopRecording(ignored_path, ignored_error);
      }
    }
    session_recording_ = false;
    session_audio_file_active_ = false;
    leaveRoom();
    setLobbyStatusText("Returned to lobby.");
    screen_navigator_.SwitchTo(ScreenMode::Lobby);
  };
  session_callbacks.on_disconnect = [this] {
    if (audio_session_service_) {
      audio_session_service_->StopAudioFilePlayback();
      juce::String ignored_path;
      juce::String ignored_error;
      if (audio_session_service_->IsRecording()) {
        audio_session_service_->StopRecording(ignored_path, ignored_error);
      }
    }
    session_recording_ = false;
    session_audio_file_active_ = false;
    leaveRoom();
    setLobbyStatusText("Disconnected from room.");
    screen_navigator_.SwitchTo(ScreenMode::Lobby);
  };
  session_callbacks.on_mute_changed = [this](bool muted) {
    if (!session_lifecycle_controller_.session_active()) return;
    deps_.session.SetMuted(muted);
    session_presence_controller_.updateSelfMuted(muted);
  };
  session_callbacks.on_input_gain_changed = [this](float gain) {
    if (session_lifecycle_controller_.session_active()) {
      deps_.session.SetInputGain(gain);
      views_.session_view.setStatusText(
          (juce::String("Input gain: ") + juce::String(gain, 2) + "x").toStdString());
    }
  };
  session_callbacks.on_input_reverb_changed = [this](float amount) {
    if (session_lifecycle_controller_.session_active()) {
      deps_.session.SetInputReverb(amount);
      views_.session_view.setStatusText(
          (juce::String("Reverb: ") + juce::String(amount, 2)).toStdString());
    }
  };
  session_callbacks.on_metronome_changed = [this](bool enabled) {
    if (enabled && session_audio_file_active_) {
      views_.session_view.setMetronomeEnabled(false);
      views_.session_view.setStatusText(
          "Audio file playback and metronome cannot run together.");
      return;
    }
    if (session_lifecycle_controller_.session_active()) {
      deps_.session.SetMetronomeEnabled(enabled);
    }
  };
  session_callbacks.on_volume_changed = [this](float volume) {
    if (session_lifecycle_controller_.session_active()) {
      deps_.session.SetVolume(volume);
    }
  };
  session_callbacks.on_audio_file_toggle = [this](bool active) {
    const bool peer_recording = std::any_of(
        session_presence_controller_.participants().begin(),
        session_presence_controller_.participants().end(),
        [](const ParticipantSummary& participant) {
          return !participant.is_self && participant.is_recording;
        });
    if (active && (session_recording_ || peer_recording)) {
      session_audio_file_active_ = false;
      views_.session_view.setAudioFileActive(false);
      views_.session_view.setStatusText(
          "녹음 중에는 오디오 파일을 재생할 수 없습니다.");
      return;
    }
    if (!audio_session_service_) {
      session_audio_file_active_ = false;
      views_.session_view.setAudioFileActive(false);
      views_.session_view.setStatusText("오디오 파일 브리지가 초기화되지 않았습니다.");
      return;
    }

    if (!active) {
      audio_session_service_->StopAudioFilePlayback();
      session_audio_file_active_ = false;
      views_.session_view.setAudioFileActive(false);
      views_.session_view.setStatusText("오디오 파일 재생을 중지했습니다.");
      return;
    }

    audio_file_chooser_ = std::make_unique<juce::FileChooser>(
        "Select audio file to share", juce::File(), "*.wav;*.mp3;*.aiff;*.flac", true,
        false, &views_.session_view);
    audio_file_chooser_->launchAsync(
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& chooser) {
          juce::String error_text;
          bool ok = false;
          const auto selected = chooser.getResult();
          if (!audio_session_service_) {
            error_text = "Audio service is not available.";
          } else if (selected.existsAsFile()) {
            ok = audio_session_service_->StartAudioFilePlayback(selected, error_text);
          } else {
            error_text = "Audio file selection cancelled.";
          }
          audio_file_chooser_.reset();
          if (!ok) {
            session_audio_file_active_ = false;
            views_.session_view.setAudioFileActive(false);
            views_.session_view.setStatusText(
                error_text.isNotEmpty()
                    ? error_text.toStdString()
                    : "오디오 파일 재생을 시작하지 못했습니다.");
            return;
          }
          session_audio_file_active_ = true;
          if (deps_.session.IsMetronomeEnabled()) {
            deps_.session.SetMetronomeEnabled(false);
            views_.session_view.setMetronomeEnabled(false);
          }
          const auto file_name = audio_session_service_->CurrentAudioFileLabel();
          views_.session_view.setStatusText(
              "오디오 파일 공유 재생 시작: " + file_name.toStdString());
        });
  };
  session_callbacks.on_audio_file_seek_changed = [this](float normalized_position) {
    if (audio_session_service_) {
      audio_session_service_->SetAudioFilePlaybackPosition(normalized_position);
    }
  };
  session_callbacks.on_audio_file_loop_changed = [this](bool enabled) {
    if (audio_session_service_) {
      audio_session_service_->SetAudioFileLoopEnabled(enabled);
      views_.session_view.setStatusText(
          enabled ? "오디오 파일 반복 재생 켜짐" : "오디오 파일 반복 재생 꺼짐");
    }
  };
  session_callbacks.on_record_toggle = [this](bool recording) {
    if (recording && session_audio_file_active_) {
      if (audio_session_service_) {
        audio_session_service_->StopAudioFilePlayback();
      }
      session_audio_file_active_ = false;
      views_.session_view.setAudioFileActive(false);
    }
    if (!audio_session_service_) {
      session_recording_ = false;
      views_.session_view.setRecording(false);
      views_.session_view.setStatusText("녹음 브리지가 초기화되지 않았습니다.");
      return;
    }
    if (recording) {
      juce::String path;
      juce::String error_text;
      if (!audio_session_service_->StartRecording(path, error_text)) {
        session_recording_ = false;
        views_.session_view.setRecording(false);
        views_.session_view.setStatusText(
            error_text.isNotEmpty() ? error_text.toStdString()
                                    : "녹음을 시작하지 못했습니다.");
        return;
      }
      session_recording_ = true;
      views_.session_view.setStatusText("녹음 시작: " + path.toStdString());
      return;
    }

    juce::String path;
    juce::String error_text;
    if (!audio_session_service_->StopRecording(path, error_text)) {
      session_recording_ = false;
      views_.session_view.setRecording(false);
      views_.session_view.setStatusText(
          error_text.isNotEmpty() ? error_text.toStdString()
                                  : "녹음을 종료하지 못했습니다.");
      return;
    }
    session_recording_ = false;
    views_.session_view.setStatusText("녹음 저장 완료: " + path.toStdString());
  };
  session_callbacks.on_bpm_changed = [this](int bpm) {
    if (session_lifecycle_controller_.session_active()) {
      deps_.session.SetMetronomeBpm(bpm);
    }
  };
  session_callbacks.on_metronome_tone_changed = [this](int tone) {
    if (session_lifecycle_controller_.session_active()) {
      deps_.session.SetMetronomeTone(tone);
    }
  };
  session_callbacks.on_input_device_changed = [this](std::string device_id) {
    if (session_lifecycle_controller_.session_active()) {
      deps_.session.SetInputDevice(device_id);
    }
  };
  session_callbacks.on_output_device_changed = [this](std::string device_id) {
    if (session_lifecycle_controller_.session_active()) {
      deps_.session.SetOutputDevice(device_id);
    }
  };
  session_callbacks.on_open_audio_settings = [this] {
    views_.session_view.setStatusText("Audio settings dialog will be connected next.");
  };
  session_callbacks.on_participant_audio_settings = [this](std::string user_id) {
    if (user_id.empty()) return;
    views_.session_view.setStatusText("Participant audio settings: " + user_id);
  };
  session_callbacks.on_participant_reconnect = [this](std::string user_id) {
    if (session_presence_controller_.sendReconnectProbe(
            user_id, session_lifecycle_controller_.session_active())) {
      views_.session_view.setStatusText("Peer reconnect probe sent: " + user_id);
    } else {
      views_.session_view.setStatusText("Peer reconnect probe failed: " + user_id);
    }
  };
  session_callbacks.on_participant_monitor_volume_changed =
      [this](std::string user_id, float volume) {
        if (user_id.empty()) return;
        session_presence_controller_.updateParticipantMonitorVolume(user_id, volume);
        if (!session_lifecycle_controller_.session_active()) return;
        const auto participant_id = tempolink::juceapp::app::HashToU32(user_id);
        deps_.session.SetPeerMonitorVolume(participant_id, volume);
      };
  session_callbacks.on_participant_monitor_pan_changed =
      [this](std::string user_id, float pan) {
        if (user_id.empty()) return;
        session_presence_controller_.updateParticipantMonitorPan(user_id, pan);
        if (!session_lifecycle_controller_.session_active()) return;
        const auto participant_id = tempolink::juceapp::app::HashToU32(user_id);
        deps_.session.SetPeerMonitorPan(participant_id, pan);
      };
  tempolink::juceapp::app::BindSessionView(views_.session_view, session_callbacks);
}
