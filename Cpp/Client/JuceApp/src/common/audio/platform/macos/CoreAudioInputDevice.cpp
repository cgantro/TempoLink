#include "tempolink/audio/platform/macos/CoreAudioInputDevice.h"

#include <chrono>
#include <thread>
#include <utility>
#include <vector>

#include "tempolink/audio/platform/macos/CoreAudioDeviceEnumerator.h"

namespace tempolink::audio {

std::vector<AudioDeviceInfo> CoreAudioInputDevice::ListDevices() const {
  return platform::macos::detail::EnumerateCoreAudioInputDevices();
}

bool CoreAudioInputDevice::SelectDevice(const std::string& device_id) {
  const auto devices = ListDevices();
  for (const auto& device : devices) {
    if (device.id == device_id || device.name == device_id) {
      selected_device_id_ = (device.id == device_id) ? device_id : device.id;
      return true;
    }
  }
  return false;
}

std::string CoreAudioInputDevice::SelectedDeviceId() const {
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

bool CoreAudioInputDevice::Start(const AudioCaptureConfig& config,
                                 AudioCaptureCallback callback) {
  if (running_.exchange(true)) {
    return false;
  }
  // Use our portable StandardThread polyfill
  capture_thread_ = util::StandardThread(
      [this, config, callback = std::move(callback)](util::StopToken token) {
        CaptureLoop(token, config, callback);
      });
  return true;
}

void CoreAudioInputDevice::Stop() {
  if (!running_.exchange(false)) {
    return;
  }
  if (capture_thread_.joinable()) {
    capture_thread_.request_stop();
    capture_thread_.join();
  }
}

bool CoreAudioInputDevice::IsRunning() const { return running_.load(); }

std::string CoreAudioInputDevice::BackendName() const { return "CoreAudio"; }

void CoreAudioInputDevice::CaptureLoop(util::StopToken stop_token,
                                       AudioCaptureConfig config,
                                       AudioCaptureCallback callback) {
  const auto frame_interval =
      std::chrono::microseconds(static_cast<std::int64_t>(
          (1000000LL * config.frame_samples) / config.sample_rate_hz));
  std::vector<float> frame(
      static_cast<std::size_t>(config.frame_samples) * config.channels, 0.0f);

  while (!stop_token.stop_requested() && running_.load()) {
    // TODO: Replace silent frame mock with actual CoreAudio capture pipeline.
    callback(frame);
    std::this_thread::sleep_for(frame_interval);
  }
}

}  // namespace tempolink::audio
