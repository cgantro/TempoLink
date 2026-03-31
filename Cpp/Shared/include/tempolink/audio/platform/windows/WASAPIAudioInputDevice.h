#pragma once

#include <atomic>
#include <string>
#include <thread>
#include <vector>

#include "tempolink/audio/IAudioInputDevice.h"

namespace tempolink::audio {

class WASAPIAudioInputDevice final : public IAudioInputDevice {
 public:
  std::vector<AudioDeviceInfo> ListDevices() const override;
  bool SelectDevice(const std::string& device_id) override;
  std::string SelectedDeviceId() const override;

  bool Start(const AudioCaptureConfig& config,
             AudioCaptureCallback callback) override;
  void Stop() override;
  bool IsRunning() const override;
  std::string BackendName() const override;

 private:
  void CaptureLoop(std::stop_token stop_token, AudioCaptureConfig config,
                   AudioCaptureCallback callback);

  std::vector<AudioDeviceInfo> devices_{
      {"wasapi-default", "WASAPI Default Input", true},
      {"wasapi-virtual-in", "WASAPI Virtual Input", false},
  };
  std::string selected_device_id_ = "wasapi-default";
  std::jthread capture_thread_;
  std::atomic_bool running_{false};
};

}  // namespace tempolink::audio
