#include "tempolink/juce/bridge/SessionMediaBridgePort.h"

#include <utility>

#include "tempolink/juce/bridge/AudioFilePlayer.h"
#include "tempolink/juce/bridge/AudioRecorder.h"

namespace tempolink::juceapp::bridge {

SessionMediaBridgePort::SessionMediaBridgePort(
    std::shared_ptr<tempolink::client::AudioBridgePort> delegate,
    std::shared_ptr<AudioFilePlayer> file_player,
    std::shared_ptr<AudioRecorder> recorder)
    : delegate_(std::move(delegate)),
      file_player_(std::move(file_player)),
      recorder_(std::move(recorder)) {}

void SessionMediaBridgePort::OnCapturedInput(
    std::span<std::int16_t> frame,
    const tempolink::audio::AudioCaptureConfig& config) {
  if (delegate_) {
    delegate_->OnCapturedInput(frame, config);
  }
  if (file_player_) {
    file_player_->MixCapturedInput(frame, config);
  }
}

void SessionMediaBridgePort::OnPlaybackOutput(
    std::span<const std::int16_t> frame,
    const tempolink::audio::AudioPlaybackConfig& config) {
  if (delegate_) {
    delegate_->OnPlaybackOutput(frame, config);
  }
  if (recorder_) {
    recorder_->ConsumePlayback(frame, config);
  }
}

}  // namespace tempolink::juceapp::bridge

