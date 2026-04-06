#pragma once

#include <vector>
#include <memory>

#include "tempolink/util/StandardThread.h"

#include "tempolink/audio/IAudioInputDevice.h"

namespace tempolink::audio {

class CoreAudioInputDevice final : public IAudioInputDevice {
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
  void CaptureLoop(util::StopToken stop_token, AudioCaptureConfig config,
                   AudioCaptureCallback callback);

  std::string selected_device_id_;
  util::StandardThread capture_thread_;
  std::atomic_bool running_{false};
};

}  // namespace tempolink::audio
