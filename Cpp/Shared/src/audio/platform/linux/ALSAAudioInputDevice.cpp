#include "tempolink/audio/platform/linux/ALSAAudioInputDevice.h"

#include <chrono>
#include <thread>
#include <utility>
#include <vector>

namespace tempolink::audio {

std::vector<AudioDeviceInfo> ALSAAudioInputDevice::ListDevices() const {
  return devices_;
}

bool ALSAAudioInputDevice::SelectDevice(const std::string& device_id) {
  for (const auto& device : devices_) {
    if (device.id == device_id) {
      selected_device_id_ = device_id;
      return true;
    }
  }
  return false;
}

std::string ALSAAudioInputDevice::SelectedDeviceId() const {
  return selected_device_id_;
}

bool ALSAAudioInputDevice::Start(const AudioCaptureConfig& config,
                                 AudioCaptureCallback callback) {
  if (running_.exchange(true)) {
    return false;
  }
  capture_thread_ = std::jthread(
      [this, config, callback = std::move(callback)](std::stop_token token) {
        CaptureLoop(token, config, callback);
      });
  return true;
}

void ALSAAudioInputDevice::Stop() {
  if (!running_.exchange(false)) {
    return;
  }
  if (capture_thread_.joinable()) {
    capture_thread_.request_stop();
    capture_thread_.join();
  }
}

bool ALSAAudioInputDevice::IsRunning() const { return running_.load(); }

std::string ALSAAudioInputDevice::BackendName() const { return "ALSA"; }

void ALSAAudioInputDevice::CaptureLoop(std::stop_token stop_token,
                                       AudioCaptureConfig config,
                                       AudioCaptureCallback callback) {
  const auto frame_interval =
      std::chrono::microseconds(static_cast<std::int64_t>(
          (1000000LL * config.frame_samples) / config.sample_rate_hz));
  std::vector<float> frame(
      static_cast<std::size_t>(config.frame_samples) * config.channels, 0.0f);

  while (!stop_token.stop_requested() && running_.load()) {
    // TODO: Replace silent frame mock with actual ALSA capture pipeline.
    callback(frame);
    std::this_thread::sleep_for(frame_interval);
  }
}

}  // namespace tempolink::audio
