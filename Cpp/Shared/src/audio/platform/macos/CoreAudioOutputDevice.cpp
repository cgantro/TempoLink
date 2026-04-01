#include "tempolink/audio/platform/macos/CoreAudioOutputDevice.h"

#include <algorithm>

#include "tempolink/audio/platform/macos/CoreAudioDeviceEnumerator.h"

namespace tempolink::audio {

std::vector<AudioDeviceInfo> CoreAudioOutputDevice::ListDevices() const {
  return platform::macos::detail::EnumerateCoreAudioOutputDevices();
}

bool CoreAudioOutputDevice::SelectDevice(const std::string& device_id) {
  const auto devices = ListDevices();
  for (const auto& device : devices) {
    if (device.id == device_id || device.name == device_id) {
      selected_device_id_ = (device.id == device_id) ? device_id : device.id;
      return true;
    }
  }
  return false;
}

std::string CoreAudioOutputDevice::SelectedDeviceId() const {
  const auto devices = ListDevices();
  for (const auto& device : devices) {
    if (device.id == selected_device_id_) {
      return selected_device_id_;
    }
  }
  for (const auto& device : devices) {
    if (device.is_default) {
      return device.id;
    }
  }
  if (!devices.empty()) {
    return devices.front().id;
  }
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
