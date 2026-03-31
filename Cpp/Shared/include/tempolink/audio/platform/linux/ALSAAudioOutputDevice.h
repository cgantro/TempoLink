#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <vector>

#include "tempolink/audio/IAudioOutputDevice.h"

namespace tempolink::audio {

class ALSAAudioOutputDevice final : public IAudioOutputDevice {
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
  void PlayFrame(std::span<const std::int16_t> pcm) override;

 private:
  std::vector<AudioDeviceInfo> devices_{
      {"alsa-default", "ALSA Default Output", true},
      {"alsa-virtual-out", "ALSA Virtual Output", false},
  };
  std::string selected_device_id_ = "alsa-default";
  std::atomic_bool running_{false};
  std::atomic<float> output_volume_{1.0F};
  std::mutex last_frame_mutex_;
  std::vector<std::int16_t> last_frame_;
  AudioPlaybackConfig config_{};
};

}  // namespace tempolink::audio

