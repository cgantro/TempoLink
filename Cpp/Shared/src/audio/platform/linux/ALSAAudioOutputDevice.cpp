#include "tempolink/audio/platform/linux/ALSAAudioOutputDevice.h"

#include <algorithm>

namespace tempolink::audio {

std::vector<AudioDeviceInfo> ALSAAudioOutputDevice::ListDevices() const {
  return devices_;
}

bool ALSAAudioOutputDevice::SelectDevice(const std::string& device_id) {
  for (const auto& device : devices_) {
    if (device.id == device_id) {
      selected_device_id_ = device_id;
      return true;
    }
  }
  return false;
}

std::string ALSAAudioOutputDevice::SelectedDeviceId() const {
  return selected_device_id_;
}

bool ALSAAudioOutputDevice::Start(const AudioPlaybackConfig& config) {
  config_ = config;
  running_ = true;
  return true;
}

void ALSAAudioOutputDevice::Stop() { running_ = false; }

bool ALSAAudioOutputDevice::IsRunning() const { return running_.load(); }

std::string ALSAAudioOutputDevice::BackendName() const { return "ALSA"; }

void ALSAAudioOutputDevice::SetOutputVolume(float gain) {
  output_volume_.store(std::clamp(gain, 0.0F, 1.0F));
}

float ALSAAudioOutputDevice::OutputVolume() const {
  return output_volume_.load();
}

void ALSAAudioOutputDevice::PlayFrame(std::span<const std::int16_t> pcm) {
  if (!running_.load()) {
    return;
  }
  std::scoped_lock lock(last_frame_mutex_);
  last_frame_.assign(pcm.begin(), pcm.end());
  // TODO: Replace frame caching with real ALSA render path.
}

}  // namespace tempolink::audio
