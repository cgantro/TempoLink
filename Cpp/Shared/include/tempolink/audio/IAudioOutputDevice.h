#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include "tempolink/audio/AudioDevice.h"

namespace tempolink::audio {

struct AudioPlaybackConfig {
  std::uint32_t sample_rate_hz = 48000;
  std::uint8_t channels = 1;
  std::uint16_t frame_samples = 480;
};

class IAudioOutputDevice {
 public:
  virtual ~IAudioOutputDevice() = default;

  virtual std::vector<AudioDeviceInfo> ListDevices() const = 0;
  virtual bool SelectDevice(const std::string& device_id) = 0;
  virtual std::string SelectedDeviceId() const = 0;

  virtual bool Start(const AudioPlaybackConfig& config) = 0;
  virtual void Stop() = 0;
  virtual bool IsRunning() const = 0;
  virtual std::string BackendName() const = 0;

  virtual void SetOutputVolume(float gain) = 0;
  virtual float OutputVolume() const = 0;
  virtual void PlayFrame(std::span<const std::int16_t> pcm) = 0;
};

}  // namespace tempolink::audio

