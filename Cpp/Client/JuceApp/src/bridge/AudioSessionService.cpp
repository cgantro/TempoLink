#include "tempolink/juce/bridge/AudioSessionService.h"

#include <utility>

#include "tempolink/juce/bridge/AudioFilePlayer.h"
#include "tempolink/juce/bridge/AudioRecorder.h"
#include "tempolink/juce/bridge/SessionMediaBridgePort.h"

namespace tempolink::juceapp::bridge {

AudioSessionService::AudioSessionService(
    std::shared_ptr<tempolink::client::AudioBridgePort> delegate)
    : player_(std::make_shared<AudioFilePlayer>()),
      recorder_(std::make_shared<AudioRecorder>()),
      bridge_port_(std::make_shared<SessionMediaBridgePort>(std::move(delegate), player_,
                                                            recorder_)) {}

std::shared_ptr<tempolink::client::AudioBridgePort> AudioSessionService::bridgePort() const {
  return bridge_port_;
}

bool AudioSessionService::StartAudioFilePlayback(const juce::File& file,
                                                 juce::String& error_text) {
  return player_->LoadFile(file, error_text);
}

void AudioSessionService::StopAudioFilePlayback() { player_->Stop(); }

bool AudioSessionService::IsAudioFilePlaybackActive() const { return player_->IsActive(); }

bool AudioSessionService::SetAudioFilePlaybackPosition(float normalized_position) {
  return player_->SetPosition(normalized_position);
}

float AudioSessionService::AudioFilePlaybackPosition() const { return player_->Position(); }

void AudioSessionService::SetAudioFileLoopEnabled(bool enabled) { player_->SetLoopEnabled(enabled); }

bool AudioSessionService::IsAudioFileLoopEnabled() const { return player_->IsLoopEnabled(); }

juce::String AudioSessionService::CurrentAudioFileLabel() const { return player_->CurrentLabel(); }

bool AudioSessionService::StartRecording(juce::String& output_path, juce::String& error_text) {
  return recorder_->Start(output_path, error_text);
}

bool AudioSessionService::StopRecording(juce::String& output_path, juce::String& error_text) {
  return recorder_->Stop(output_path, error_text);
}

bool AudioSessionService::IsRecording() const { return recorder_->IsRecording(); }

}  // namespace tempolink::juceapp::bridge

