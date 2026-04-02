#include "tempolink/audio/platform/generic/NullAudioOutputDevice.h"

#include <algorithm>

namespace tempolink::audio {

std::vector<AudioDeviceInfo> NullAudioOutputDevice::ListDevices() const {
  return devices_;
}

bool NullAudioOutputDevice::SelectDevice(const std::string& device_id) {
  for (const auto& device : devices_) {
    if (device.id == device_id) {
      selected_device_id_ = device_id;
      return true;
    }
  }
  return false;
}

std::string NullAudioOutputDevice::SelectedDeviceId() const {
  return selected_device_id_;
}

bool NullAudioOutputDevice::Start(const AudioPlaybackConfig& config) {
  config_ = config;
  running_ = true;
  return true;
}

void NullAudioOutputDevice::Stop() { running_ = false; }

bool NullAudioOutputDevice::IsRunning() const { return running_.load(); }

std::string NullAudioOutputDevice::BackendName() const { return "NullAudio"; }

void NullAudioOutputDevice::SetOutputVolume(float gain) {
  output_volume_.store(std::clamp(gain, 0.0F, 1.0F));
}

float NullAudioOutputDevice::OutputVolume() const {
  return output_volume_.load();
}

void NullAudioOutputDevice::PlayFrame(std::span<const float> pcm) {
  if (!running_.load()) {
    return;
  }
  std::scoped_lock lock(last_frame_mutex_);
  last_frame_.assign(pcm.begin(), pcm.end());
}

}  // namespace tempolink::audio
