#include "tempolink/client/AudioPipeline.h"

#include <algorithm>
#include <cmath>
#include <mutex>
#include <vector>

#include "tempolink/audio/IAudioInputDevice.h"
#include "tempolink/audio/IAudioOutputDevice.h"
#include "tempolink/audio/AudioInputFactory.h"
#include "tempolink/audio/AudioOutputFactory.h"
#include "tempolink/config/AudioConstants.h"

namespace tempolink::client {

namespace {

float ComputeNormalizedPeak(std::span<const float> pcm) {
  if (pcm.empty()) {
    return 0.0F;
  }

  float max_abs = 0.0f;
  for (const auto sample : pcm) {
    const auto abs_sample = std::abs(sample);
    if (abs_sample > max_abs) {
      max_abs = abs_sample;
    }
  }

  return std::clamp(max_abs, 0.0F, 1.0F);
}

float SmoothLevel(float previous, float incoming) {
  constexpr float kAttack = 0.15F;
  constexpr float kRelease = 0.05F;
  if (incoming > previous) {
    return previous + (incoming - previous) * kAttack;
  }
  return previous + (incoming - previous) * kRelease;
}

std::vector<std::string> ToDeviceIds(const std::vector<tempolink::audio::AudioDeviceInfo>& devices) {
  std::vector<std::string> ids;
  ids.reserve(devices.size());
  for (const auto& device : devices) {
    ids.push_back(device.id);
  }
  return ids;
}

std::string ToDisplayDevice(const std::vector<tempolink::audio::AudioDeviceInfo>& devices,
                            const std::string& selected_id) {
  for (const auto& device : devices) {
    if (device.id == selected_id) {
      return device.name;
    }
  }
  return selected_id;
}

}  // namespace

bool AudioPipeline::Start(CapturedPcmCallback on_pcm_captured) {
  Stop();

  audio_input_ = tempolink::audio::CreateDefaultAudioInputDevice();
  audio_output_ = tempolink::audio::CreateDefaultAudioOutputDevice();

  if (!audio_input_ || !audio_output_) {
    Stop();
    return false;
  }

  if (!selected_input_device_.empty()) {
    audio_input_->SelectDevice(selected_input_device_);
  } else {
    selected_input_device_ = audio_input_->SelectedDeviceId();
  }

  if (!selected_output_device_.empty()) {
    audio_output_->SelectDevice(selected_output_device_);
  } else {
    selected_output_device_ = audio_output_->SelectedDeviceId();
  }

  if (!audio_output_->Start(playback_config_)) {
    Stop();
    return false;
  }
  audio_output_->SetOutputVolume(volume_.load());

  {
    std::scoped_lock lock(callback_mutex_);
    captured_pcm_callback_ = std::move(on_pcm_captured);
  }
  running_ = true;
  
  reverb_processor_.Reset();

  if (!audio_input_->Start(
          capture_config_, [this](std::span<const float> pcm) {
            OnCapturedFrame(pcm);
          })) {
    Stop();
    return false;
  }

  return true;
}

void AudioPipeline::Stop() {
  running_ = false;
  if (audio_input_) {
    audio_input_->Stop();
  }
  if (audio_output_) {
    audio_output_->Stop();
  }

  {
    std::scoped_lock lock(callback_mutex_);
    captured_pcm_callback_ = nullptr;
  }
  audio_input_.reset();
  audio_output_.reset();
}

bool AudioPipeline::IsRunning() const { return running_.load(); }

void AudioPipeline::SetMuted(bool muted) { muted_.store(muted); }

bool AudioPipeline::IsMuted() const { return muted_.load(); }

void AudioPipeline::SetInputGain(float gain) {
  input_gain_.store(std::clamp(gain, 0.0F, 2.0F));
}

float AudioPipeline::InputGain() const { return input_gain_.load(); }

void AudioPipeline::SetInputReverb(float amount) {
  reverb_processor_.SetAmount(amount);
}

float AudioPipeline::InputReverb() const { return reverb_processor_.Amount(); }

void AudioPipeline::SetVolume(float volume) {
  const float clamped = std::clamp(volume, 0.0F, 1.0F);
  volume_.store(clamped);
  if (audio_output_) {
    audio_output_->SetOutputVolume(clamped);
  }
}

float AudioPipeline::Volume() const { return volume_.load(); }

float AudioPipeline::InputLevel() const { return input_level_.load(); }

float AudioPipeline::OutputLevel() const { return output_level_.load(); }

std::string AudioPipeline::AudioBackendName() const {
  std::string input_backend = "input-uninitialized";
  std::string output_backend = "output-uninitialized";
  if (audio_input_) {
    input_backend = audio_input_->BackendName();
  }
  if (audio_output_) {
    output_backend = audio_output_->BackendName();
  }
  return input_backend + " / " + output_backend;
}

std::vector<std::string> AudioPipeline::AvailableInputDevices() const {
  if (audio_input_) {
    return ToDeviceIds(audio_input_->ListDevices());
  }
  auto temp_input = tempolink::audio::CreateDefaultAudioInputDevice();
  if (!temp_input) {
    return {};
  }
  return ToDeviceIds(temp_input->ListDevices());
}

std::vector<std::string> AudioPipeline::AvailableOutputDevices() const {
  if (audio_output_) {
    return ToDeviceIds(audio_output_->ListDevices());
  }
  auto temp_output = tempolink::audio::CreateDefaultAudioOutputDevice();
  if (!temp_output) {
    return {};
  }
  return ToDeviceIds(temp_output->ListDevices());
}

bool AudioPipeline::SetInputDevice(const std::string& device_id) {
  selected_input_device_ = device_id;
  if (audio_input_) {
    return audio_input_->SelectDevice(device_id);
  }
  return true;
}

bool AudioPipeline::SetOutputDevice(const std::string& device_id) {
  selected_output_device_ = device_id;
  if (audio_output_) {
    return audio_output_->SelectDevice(device_id);
  }
  return true;
}

std::string AudioPipeline::SelectedInputDevice() const {
  if (audio_input_) {
    return ToDisplayDevice(audio_input_->ListDevices(), audio_input_->SelectedDeviceId());
  }
  return selected_input_device_;
}

std::string AudioPipeline::SelectedOutputDevice() const {
  if (audio_output_) {
    return ToDisplayDevice(audio_output_->ListDevices(), audio_output_->SelectedDeviceId());
  }
  return selected_output_device_;
}

bool AudioPipeline::ConfigureAudioFormat(std::uint32_t sample_rate_hz,
                                         std::uint16_t frame_samples) {
  if (!tempolink::config::IsSupportedSampleRate(sample_rate_hz) ||
      !tempolink::config::IsSupportedFrameSamples(frame_samples)) {
    return false;
  }

  capture_config_.sample_rate_hz = sample_rate_hz;
  playback_config_.sample_rate_hz = sample_rate_hz;
  capture_config_.frame_samples = frame_samples;
  playback_config_.frame_samples = frame_samples;

  if (!running_.load()) {
    return true;
  }

  if (!audio_input_ || !audio_output_) {
    return false;
  }

  audio_input_->Stop();
  audio_output_->Stop();

  if (!audio_output_->Start(playback_config_)) {
    Stop();
    return false;
  }
  audio_output_->SetOutputVolume(volume_.load());
  if (!audio_input_->Start(
          capture_config_, [this](std::span<const float> pcm) {
            OnCapturedFrame(pcm);
          })) {
    Stop();
    return false;
  }
  
  reverb_processor_.Reset();
  
  return true;
}

std::uint32_t AudioPipeline::SampleRateHz() const {
  return capture_config_.sample_rate_hz;
}

std::uint16_t AudioPipeline::FrameSamples() const {
  return capture_config_.frame_samples;
}

void AudioPipeline::SetAudioBridge(std::shared_ptr<AudioBridgePort> audio_bridge) {
  audio_bridge_ = std::move(audio_bridge);
}

void AudioPipeline::SetMetronomeEnabled(bool enabled) {
  metronome_processor_.SetEnabled(enabled);
}

bool AudioPipeline::IsMetronomeEnabled() const {
  return metronome_processor_.IsEnabled();
}

void AudioPipeline::SetMetronomeBpm(int bpm) {
  metronome_processor_.SetBpm(bpm);
}

int AudioPipeline::MetronomeBpm() const {
  return metronome_processor_.Bpm();
}

void AudioPipeline::SetMetronomeVolume(float volume) {
  metronome_processor_.SetVolume(volume);
}

float AudioPipeline::MetronomeVolume() const {
  return metronome_processor_.Volume();
}

void AudioPipeline::SetPeerMonitorVolume(std::uint32_t participant_id, float volume) {
  peer_mixer_.SetPeerVolume(participant_id, volume);
}

float AudioPipeline::PeerMonitorVolume(std::uint32_t participant_id) const {
  return peer_mixer_.PeerVolume(participant_id);
}

void AudioPipeline::SetPeerMonitorPan(std::uint32_t participant_id, float pan) {
  peer_mixer_.SetPeerPan(participant_id, pan);
}

float AudioPipeline::PeerMonitorPan(std::uint32_t participant_id) const {
  return peer_mixer_.PeerPan(participant_id);
}

void AudioPipeline::HandleIncomingAudio(std::uint32_t sender_participant_id,
                                        std::span<const float> pcm) {
  if (!running_.load() || !audio_output_) {
    return;
  }

  // Master output buffer for this frame
  std::vector<float> mixed(pcm.size(), 0.0f);
  
  audio::AudioFormat format;
  format.sample_rate_hz = playback_config_.sample_rate_hz;
  format.channels = playback_config_.channels;
  format.frame_samples = playback_config_.frame_samples;

  // Mix this peer into the master buffer
  peer_mixer_.MixPeer(mixed, pcm, sender_participant_id, format);

  const float output_peak = ComputeNormalizedPeak(mixed);
  const float prev_output_level = output_level_.load();
  output_level_.store(SmoothLevel(prev_output_level, output_peak));

  audio_output_->PlayFrame(mixed);
}

void AudioPipeline::OnCapturedFrame(std::span<const float> pcm) {
  if (!running_.load()) {
    return;
  }

  const std::size_t channels = std::max<std::size_t>(1, capture_config_.channels);
  const std::size_t expected_samples =
      static_cast<std::size_t>(capture_config_.frame_samples) * channels;
  if (expected_samples == 0 || pcm.size() < expected_samples) {
    return;
  }

  if (muted_.load()) {
    const float prev_input_level = input_level_.load();
    input_level_.store(SmoothLevel(prev_input_level, 0.0F));
    return;
  }

  std::vector<float> processed(pcm.begin(), pcm.begin() + expected_samples);
  const float input_gain = input_gain_.load();
  if (std::abs(input_gain - 1.0F) > 0.0001F) {
    for (auto& sample : processed) {
      sample *= input_gain;
    }
  }

  audio::AudioFormat format;
  format.sample_rate_hz = capture_config_.sample_rate_hz;
  format.channels = capture_config_.channels;
  format.frame_samples = capture_config_.frame_samples;

  // Apply modular processors
  reverb_processor_.Process(processed, format);
  metronome_processor_.Process(processed, format);

  const float input_peak = ComputeNormalizedPeak(processed);
  const float prev_input_level = input_level_.load();
  input_level_.store(SmoothLevel(prev_input_level, input_peak));

  CapturedPcmCallback callback;
  {
    std::scoped_lock lock(callback_mutex_);
    callback = captured_pcm_callback_;
  }
  if (callback) {
    callback(processed);
  }
}

}  // namespace tempolink::client
