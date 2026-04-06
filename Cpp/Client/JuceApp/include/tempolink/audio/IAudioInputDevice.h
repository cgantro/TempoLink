#pragma once

#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <vector>

#include "tempolink/audio/AudioDevice.h"

namespace tempolink::audio {

struct AudioCaptureConfig {
  std::uint32_t sample_rate_hz = 48000;
  std::uint8_t channels = 1;
  std::uint16_t frame_samples = 480;  // 10ms @ 48kHz
};

using AudioCaptureCallback = std::function<void(std::span<const float>)>;

class IAudioInputDevice {
 public:
  virtual ~IAudioInputDevice() = default;

  virtual std::vector<AudioDeviceInfo> ListDevices() const = 0;
  virtual bool SelectDevice(const std::string& device_id) = 0;
  virtual std::string SelectedDeviceId() const = 0;

  virtual bool Start(const AudioCaptureConfig& config,
                     AudioCaptureCallback callback) = 0;
  virtual void Stop() = 0;
  virtual bool IsRunning() const = 0;
  virtual std::string BackendName() const = 0;
};

}  // namespace tempolink::audio
