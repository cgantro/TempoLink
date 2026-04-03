#include "tempolink/client/audio/AudioDeviceManager.h"

#include "tempolink/audio/AudioInputFactory.h"
#include "tempolink/audio/AudioOutputFactory.h"

namespace tempolink::client::audio {

namespace {

std::string ToDisplayName(const tempolink::audio::AudioDeviceInfo& device) {
  return device.name.empty() ? device.id : device.name;
}

std::vector<std::string> ToDisplayNames(
    const std::vector<tempolink::audio::AudioDeviceInfo>& devices) {
  std::vector<std::string> names;
  names.reserve(devices.size());
  for (const auto& d : devices) {
    names.push_back(ToDisplayName(d));
  }
  return names;
}

std::string ResolveDeviceId(const std::vector<tempolink::audio::AudioDeviceInfo>& devices,
                            const std::string& device_label_or_id) {
  for (const auto& d : devices) {
    if (d.id == device_label_or_id || d.name == device_label_or_id) {
      return d.id;
    }
  }
  return device_label_or_id;
}

std::string ToSelectedDisplay(const std::vector<tempolink::audio::AudioDeviceInfo>& devices,
                              const std::string& selected_id_or_name) {
  for (const auto& d : devices) {
    if (d.id == selected_id_or_name || d.name == selected_id_or_name) {
      return ToDisplayName(d);
    }
  }
  return selected_id_or_name;
}

}  // namespace

bool AudioDeviceManager::CreateDevices() {
  input_ = tempolink::audio::CreateDefaultAudioInputDevice();
  output_ = tempolink::audio::CreateDefaultAudioOutputDevice();
  if (!input_ || !output_) {
    DestroyDevices();
    return false;
  }

  if (!preferred_input_device_.empty()) {
    input_->SelectDevice(preferred_input_device_);
  } else {
    preferred_input_device_ = input_->SelectedDeviceId();
  }

  if (!preferred_output_device_.empty()) {
    output_->SelectDevice(preferred_output_device_);
  } else {
    preferred_output_device_ = output_->SelectedDeviceId();
  }

  return true;
}

void AudioDeviceManager::DestroyDevices() {
  if (input_) input_->Stop();
  if (output_) output_->Stop();
  input_.reset();
  output_.reset();
}

bool AudioDeviceManager::StartInput(const tempolink::audio::AudioCaptureConfig& config,
                                    tempolink::audio::AudioCaptureCallback callback) {
  if (!input_) return false;
  return input_->Start(config, std::move(callback));
}

void AudioDeviceManager::StopInput() {
  if (input_) input_->Stop();
}

std::vector<std::string> AudioDeviceManager::AvailableInputDevices() const {
  if (input_) return ToDisplayNames(input_->ListDevices());
  auto temp = tempolink::audio::CreateDefaultAudioInputDevice();
  if (!temp) return {};
  return ToDisplayNames(temp->ListDevices());
}

bool AudioDeviceManager::SelectInputDevice(const std::string& device_id) {
  std::vector<tempolink::audio::AudioDeviceInfo> devices;
  if (input_) {
    devices = input_->ListDevices();
  } else {
    auto temp = tempolink::audio::CreateDefaultAudioInputDevice();
    if (temp) {
      devices = temp->ListDevices();
    }
  }
  preferred_input_device_ = ResolveDeviceId(devices, device_id);
  if (input_) return input_->SelectDevice(preferred_input_device_);
  return true;
}

std::string AudioDeviceManager::SelectedInputDevice() const {
  if (input_) return ToSelectedDisplay(input_->ListDevices(), input_->SelectedDeviceId());
  auto temp = tempolink::audio::CreateDefaultAudioInputDevice();
  if (temp) {
    return ToSelectedDisplay(temp->ListDevices(), preferred_input_device_);
  }
  return preferred_input_device_;
}

std::string AudioDeviceManager::InputBackendName() const {
  return input_ ? input_->BackendName() : "input-uninitialized";
}

bool AudioDeviceManager::StartOutput(const tempolink::audio::AudioPlaybackConfig& config) {
  if (!output_) return false;
  return output_->Start(config);
}

void AudioDeviceManager::StopOutput() {
  if (output_) output_->Stop();
}

std::vector<std::string> AudioDeviceManager::AvailableOutputDevices() const {
  if (output_) return ToDisplayNames(output_->ListDevices());
  auto temp = tempolink::audio::CreateDefaultAudioOutputDevice();
  if (!temp) return {};
  return ToDisplayNames(temp->ListDevices());
}

bool AudioDeviceManager::SelectOutputDevice(const std::string& device_id) {
  std::vector<tempolink::audio::AudioDeviceInfo> devices;
  if (output_) {
    devices = output_->ListDevices();
  } else {
    auto temp = tempolink::audio::CreateDefaultAudioOutputDevice();
    if (temp) {
      devices = temp->ListDevices();
    }
  }
  preferred_output_device_ = ResolveDeviceId(devices, device_id);
  if (output_) return output_->SelectDevice(preferred_output_device_);
  return true;
}

std::string AudioDeviceManager::SelectedOutputDevice() const {
  if (output_) return ToSelectedDisplay(output_->ListDevices(), output_->SelectedDeviceId());
  auto temp = tempolink::audio::CreateDefaultAudioOutputDevice();
  if (temp) {
    return ToSelectedDisplay(temp->ListDevices(), preferred_output_device_);
  }
  return preferred_output_device_;
}

std::string AudioDeviceManager::OutputBackendName() const {
  return output_ ? output_->BackendName() : "output-uninitialized";
}

void AudioDeviceManager::SetOutputVolume(float volume) {
  if (output_) output_->SetOutputVolume(volume);
}

void AudioDeviceManager::PlayFrame(std::span<const float> pcm) {
  if (output_) output_->PlayFrame(pcm);
}

bool AudioDeviceManager::HasInput() const { return input_ != nullptr; }
bool AudioDeviceManager::HasOutput() const { return output_ != nullptr; }

}  // namespace tempolink::client::audio
