#include "tempolink/audio/platform/windows/WASAPIAudioOutputDevice.h"

#include <algorithm>
#include <cstring>

namespace tempolink::audio {

std::vector<AudioDeviceInfo> WASAPIAudioOutputDevice::ListDevices() const {
  return devices_;
}

bool WASAPIAudioOutputDevice::SelectDevice(const std::string& device_id) {
  for (const auto& device : devices_) {
    if (device.id == device_id) {
      selected_device_id_ = device_id;
      return true;
    }
  }
  return false;
}

std::string WASAPIAudioOutputDevice::SelectedDeviceId() const {
  return selected_device_id_;
}

bool WASAPIAudioOutputDevice::Start(const AudioPlaybackConfig& config) {
  config_ = config;
  running_ = true;
  return true;
}

void WASAPIAudioOutputDevice::Stop() { running_ = false; }

bool WASAPIAudioOutputDevice::IsRunning() const { return running_.load(); }

std::string WASAPIAudioOutputDevice::BackendName() const { return "WASAPI-MOCK"; }

void WASAPIAudioOutputDevice::SetOutputVolume(float gain) {
  output_volume_.store(std::clamp(gain, 0.0F, 1.0F));
}

float WASAPIAudioOutputDevice::OutputVolume() const {
  return output_volume_.load();
}

void WASAPIAudioOutputDevice::PlayFrame(std::span<const std::int16_t> pcm) {
  if (!running_.load()) {
    return;
  }
  std::scoped_lock lock(last_frame_mutex_);
  last_frame_.assign(pcm.begin(), pcm.end());
  // TODO: Replace frame caching with real WASAPI render path.
}

}  // namespace tempolink::audio
