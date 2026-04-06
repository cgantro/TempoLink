#pragma once

#include <atomic>
#include <deque>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "tempolink/audio/IAudioOutputDevice.h"

namespace tempolink::audio {

class WASAPIAudioOutputDevice final : public IAudioOutputDevice {
 public:
  std::vector<AudioDeviceInfo> ListDevices() const override;
  bool SelectDevice(const std::string& device_id) override;
  std::string SelectedDeviceId() const override;

  bool Start(const AudioPlaybackConfig& config) override;
  void Stop() override;
  bool IsRunning() const override;
  std::string BackendName() const override;

  void SetOutputVolume(float gain) override;
  float OutputVolume() const override;
  void PlayFrame(std::span<const float> pcm) override;

 private:
  void RenderLoop(std::stop_token stop_token);

  std::string selected_device_id_;
  AudioPlaybackConfig config_{};
  std::atomic_bool running_{false};
  std::atomic<float> output_volume_{1.0F};
  std::jthread render_thread_;
  mutable std::mutex queue_mutex_;
  std::deque<float> pending_samples_;
};

}  // namespace tempolink::audio
