#pragma once

#include <memory>
#include <span>

#include "tempolink/client/AudioBridgePort.h"

namespace tempolink::juceapp::bridge {

class AudioFilePlayer;
class AudioRecorder;

class SessionMediaBridgePort final : public tempolink::client::AudioBridgePort {
 public:
  SessionMediaBridgePort(
      std::shared_ptr<tempolink::client::AudioBridgePort> delegate,
      std::shared_ptr<AudioFilePlayer> file_player,
      std::shared_ptr<AudioRecorder> recorder);

  void OnCapturedInput(std::span<std::int16_t> frame,
                       const tempolink::audio::AudioCaptureConfig& config) override;

  void OnPlaybackOutput(std::span<const std::int16_t> frame,
                        const tempolink::audio::AudioPlaybackConfig& config) override;

 private:
  std::shared_ptr<tempolink::client::AudioBridgePort> delegate_;
  std::shared_ptr<AudioFilePlayer> file_player_;
  std::shared_ptr<AudioRecorder> recorder_;
};

}  // namespace tempolink::juceapp::bridge

