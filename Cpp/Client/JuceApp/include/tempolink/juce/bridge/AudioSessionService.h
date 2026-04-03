#pragma once

#include <memory>

#include <juce_core/juce_core.h>

#include "tempolink/client/AudioBridgePort.h"

namespace tempolink::juceapp::bridge {

class AudioFilePlayer;
class AudioRecorder;
class SessionMediaBridgePort;

class AudioSessionService final {
 public:
  explicit AudioSessionService(
      std::shared_ptr<tempolink::client::AudioBridgePort> delegate);

  std::shared_ptr<tempolink::client::AudioBridgePort> bridgePort() const;

  bool StartAudioFilePlayback(const juce::File& file, juce::String& error_text);
  void StopAudioFilePlayback();
  bool IsAudioFilePlaybackActive() const;
  bool SetAudioFilePlaybackPosition(float normalized_position);
  float AudioFilePlaybackPosition() const;
  void SetAudioFileLoopEnabled(bool enabled);
  bool IsAudioFileLoopEnabled() const;
  juce::String CurrentAudioFileLabel() const;

  bool StartRecording(juce::String& output_path, juce::String& error_text);
  bool StopRecording(juce::String& output_path, juce::String& error_text);
  bool IsRecording() const;

 private:
  std::shared_ptr<AudioFilePlayer> player_;
  std::shared_ptr<AudioRecorder> recorder_;
  std::shared_ptr<SessionMediaBridgePort> bridge_port_;
};

}  // namespace tempolink::juceapp::bridge

