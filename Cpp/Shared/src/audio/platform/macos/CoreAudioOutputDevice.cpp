#include "tempolink/audio/platform/macos/CoreAudioOutputDevice.h"

#include <algorithm>

namespace tempolink::audio {

std::vector<AudioDeviceInfo> CoreAudioOutputDevice::ListDevices() const {
  return devices_;
}

bool CoreAudioOutputDevice::SelectDevice(const std::string& device_id) {
  for (const auto& device : devices_) {
    if (device.id == device_id) {
      selected_device_id_ = device_id;
      return true;
    }
  }
  return false;
}

std::string CoreAudioOutputDevice::SelectedDeviceId() const {
  return selected_device_id_;
}

bool CoreAudioOutputDevice::Start(const AudioPlaybackConfig& config) {
  config_ = config;
  running_ = true;
  return true;
}

void CoreAudioOutputDevice::Stop() { running_ = false; }

bool CoreAudioOutputDevice::IsRunning() const { return running_.load(); }

std::string CoreAudioOutputDevice::BackendName() const { return "CoreAudio"; }

void CoreAudioOutputDevice::SetOutputVolume(float gain) {
  output_volume_.store(std::clamp(gain, 0.0F, 1.0F));
}

float CoreAudioOutputDevice::OutputVolume() const {
  return output_volume_.load();
}

void CoreAudioOutputDevice::PlayFrame(std::span<const std::int16_t> pcm) {
  if (!running_.load()) {
    return;
  }
  std::scoped_lock lock(last_frame_mutex_);
  last_frame_.assign(pcm.begin(), pcm.end());
  // TODO: Replace frame caching with real CoreAudio render path.
}

}  // namespace tempolink::audio
