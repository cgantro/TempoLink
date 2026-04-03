#pragma once

#include <memory>
#include <string>
#include <vector>

#include "tempolink/audio/IAudioInputDevice.h"
#include "tempolink/audio/IAudioOutputDevice.h"

namespace tempolink::client::audio {

/// Manages audio device lifecycle, creation, selection, and format configuration.
/// Extracted from AudioPipeline to enforce SRP.
class AudioDeviceManager {
 public:
  bool CreateDevices();
  void DestroyDevices();

  // --- Input device ---
  bool StartInput(const tempolink::audio::AudioCaptureConfig& config,
                  tempolink::audio::AudioCaptureCallback callback);
  void StopInput();
  std::vector<std::string> AvailableInputDevices() const;
  bool SelectInputDevice(const std::string& device_id);
  std::string SelectedInputDevice() const;
  std::string InputBackendName() const;

  // --- Output device ---
  bool StartOutput(const tempolink::audio::AudioPlaybackConfig& config);
  void StopOutput();
  std::vector<std::string> AvailableOutputDevices() const;
  bool SelectOutputDevice(const std::string& device_id);
  std::string SelectedOutputDevice() const;
  std::string OutputBackendName() const;

  void SetOutputVolume(float volume);
  void PlayFrame(std::span<const float> pcm);

  bool HasInput() const;
  bool HasOutput() const;

 private:
  std::unique_ptr<tempolink::audio::IAudioInputDevice> input_;
  std::unique_ptr<tempolink::audio::IAudioOutputDevice> output_;
  std::string preferred_input_device_;
  std::string preferred_output_device_;
};

}  // namespace tempolink::client::audio
