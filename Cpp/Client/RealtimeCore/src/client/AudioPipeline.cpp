#include "tempolink/client/AudioPipeline.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "tempolink/audio/AudioCodecFactory.h"
#include "tempolink/audio/AudioInputFactory.h"
#include "tempolink/audio/AudioOutputFactory.h"
#include "tempolink/config/AudioConstants.h"

namespace tempolink::client {
namespace {

std::vector<std::string> ToDeviceIds(
    const std::vector<tempolink::audio::AudioDeviceInfo>& devices) {
  std::vector<std::string> ids;
  ids.reserve(devices.size());
  for (const auto& device : devices) {
    ids.push_back(device.id);
  }
  return ids;
}

std::int16_t ClampPcm(float sample) {
  constexpr float kMin = -32768.0F;
  constexpr float kMax = 32767.0F;
  const float clamped = std::clamp(sample, kMin, kMax);
  return static_cast<std::int16_t>(clamped);
}

bool InitializeCodecPair(
    std::unique_ptr<tempolink::audio::IAudioCodec>& encoder,
    std::unique_ptr<tempolink::audio::IAudioCodec>& decoder,
    std::uint32_t sample_rate_hz, std::uint8_t channels, std::uint16_t frame_samples) {
  if (encoder != nullptr && decoder != nullptr &&
      encoder->Initialize(sample_rate_hz, channels, frame_samples) &&
      decoder->Initialize(sample_rate_hz, channels, frame_samples)) {
    return true;
  }

  // Fallback for high sample-rate interfaces where low-delay Opus init can fail.
  encoder = tempolink::audio::CreatePcmAudioCodec();
  decoder = tempolink::audio::CreatePcmAudioCodec();
  return encoder != nullptr && decoder != nullptr &&
         encoder->Initialize(sample_rate_hz, channels, frame_samples) &&
         decoder->Initialize(sample_rate_hz, channels, frame_samples);
}

}  // namespace

bool AudioPipeline::Start(EncodedFrameCallback on_frame_encoded) {
  Stop();

  audio_input_ = tempolink::audio::CreateDefaultAudioInputDevice();
  audio_output_ = tempolink::audio::CreateDefaultAudioOutputDevice();
  audio_encoder_ = tempolink::audio::CreateDefaultAudioCodec();
  audio_decoder_ = tempolink::audio::CreateDefaultAudioCodec();

  if (!audio_input_ || !audio_output_ || !audio_encoder_ || !audio_decoder_) {
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

  if (!InitializeCodecPair(audio_encoder_, audio_decoder_, capture_config_.sample_rate_hz,
                           capture_config_.channels, capture_config_.frame_samples)) {
    Stop();
    return false;
  }

  if (!audio_output_->Start(playback_config_)) {
    Stop();
    return false;
  }
  audio_output_->SetOutputVolume(volume_.load());

  {
    std::scoped_lock lock(callback_mutex_);
    encoded_frame_callback_ = std::move(on_frame_encoded);
  }
  running_ = true;
  metronome_phase_samples_ = 0;
  metronome_beat_index_ = 0;

  if (!audio_input_->Start(
          capture_config_, [this](std::span<const std::int16_t> pcm) {
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
    encoded_frame_callback_ = nullptr;
  }
  audio_input_.reset();
  audio_output_.reset();
  audio_encoder_.reset();
  audio_decoder_.reset();
  metronome_phase_samples_ = 0;
  metronome_beat_index_ = 0;
}

bool AudioPipeline::IsRunning() const { return running_.load(); }

void AudioPipeline::SetMuted(bool muted) { muted_.store(muted); }

bool AudioPipeline::IsMuted() const { return muted_.load(); }

void AudioPipeline::SetVolume(float volume) {
  const float clamped = std::clamp(volume, 0.0F, 1.0F);
  volume_.store(clamped);
  if (audio_output_) {
    audio_output_->SetOutputVolume(clamped);
  }
}

float AudioPipeline::Volume() const { return volume_.load(); }

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
    return audio_input_->SelectedDeviceId();
  }
  return selected_input_device_;
}

std::string AudioPipeline::SelectedOutputDevice() const {
  if (audio_output_) {
    return audio_output_->SelectedDeviceId();
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

  if (!audio_input_ || !audio_output_ || !audio_encoder_ || !audio_decoder_) {
    return false;
  }

  audio_input_->Stop();
  audio_output_->Stop();

  if (!InitializeCodecPair(audio_encoder_, audio_decoder_, capture_config_.sample_rate_hz,
                           capture_config_.channels, capture_config_.frame_samples)) {
    Stop();
    return false;
  }
  if (!audio_output_->Start(playback_config_)) {
    Stop();
    return false;
  }
  audio_output_->SetOutputVolume(volume_.load());
  if (!audio_input_->Start(
          capture_config_, [this](std::span<const std::int16_t> pcm) {
            OnCapturedFrame(pcm);
          })) {
    Stop();
    return false;
  }
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
  metronome_enabled_.store(enabled);
}

bool AudioPipeline::IsMetronomeEnabled() const {
  return metronome_enabled_.load();
}

void AudioPipeline::SetMetronomeBpm(int bpm) {
  metronome_bpm_.store(std::clamp(bpm, 30, 300));
}

int AudioPipeline::MetronomeBpm() const { return metronome_bpm_.load(); }

void AudioPipeline::SetMetronomeVolume(float volume) {
  metronome_volume_.store(std::clamp(volume, 0.0F, 1.0F));
}

float AudioPipeline::MetronomeVolume() const { return metronome_volume_.load(); }

void AudioPipeline::HandleIncomingAudio(
    std::span<const std::byte> encoded_payload) {
  if (!running_.load() || !audio_decoder_ || !audio_output_) {
    return;
  }

  auto decoded = audio_decoder_->Decode(encoded_payload);
  if (decoded.empty()) {
    return;
  }

  const float gain = volume_.load();
  for (auto& sample : decoded) {
    sample = ClampPcm(static_cast<float>(sample) * gain);
  }

  auto bridge = audio_bridge_;
  if (bridge != nullptr) {
    bridge->OnPlaybackOutput(
        std::span<const std::int16_t>(decoded.data(), decoded.size()),
        playback_config_);
  }
  audio_output_->PlayFrame(decoded);
}

void AudioPipeline::OnCapturedFrame(std::span<const std::int16_t> pcm) {
  if (!running_.load() || muted_.load() || !audio_encoder_) {
    return;
  }

  const std::size_t channels = std::max<std::size_t>(1, capture_config_.channels);
  const std::size_t expected_samples =
      static_cast<std::size_t>(capture_config_.frame_samples) * channels;
  if (expected_samples == 0 || pcm.size() < expected_samples) {
    return;
  }

  std::vector<std::int16_t> mixed(pcm.begin(), pcm.begin() + expected_samples);
  MixMetronomeClick(mixed);

  auto bridge = audio_bridge_;
  if (bridge != nullptr) {
    bridge->OnCapturedInput(std::span<std::int16_t>(mixed.data(), mixed.size()),
                            capture_config_);
  }

  auto encoded = audio_encoder_->Encode(mixed);
  if (encoded.empty()) {
    return;
  }
  EncodedFrameCallback callback;
  {
    std::scoped_lock lock(callback_mutex_);
    callback = encoded_frame_callback_;
  }
  if (!callback) {
    return;
  }
  callback(encoded);
}

void AudioPipeline::MixMetronomeClick(std::vector<std::int16_t>& pcm) {
  if (!metronome_enabled_.load()) {
    return;
  }

  const int bpm = std::clamp(metronome_bpm_.load(), 30, 300);
  const std::uint64_t samples_per_beat =
      (static_cast<std::uint64_t>(capture_config_.sample_rate_hz) * 60ULL) /
      static_cast<std::uint64_t>(bpm);
  if (samples_per_beat == 0) {
    return;
  }

  const std::size_t channels = std::max<std::size_t>(1, capture_config_.channels);
  const std::size_t frame_samples = pcm.size() / channels;
  if (frame_samples == 0) {
    return;
  }

  const float metronome_gain = metronome_volume_.load();
  constexpr float kTwoPi = 6.28318530717958647692F;

  for (std::size_t sample_index = 0; sample_index < frame_samples;
       ++sample_index) {
    if (metronome_phase_samples_ == 0) {
      const bool accent = (metronome_beat_index_ % 4U) == 0U;
      const float accent_gain = accent ? 1.0F : 0.7F;
      const float frequency_hz = accent ? 1760.0F : 1320.0F;
      const std::size_t click_len =
          static_cast<std::size_t>(capture_config_.sample_rate_hz / 90U);

      for (std::size_t k = 0; k < click_len && sample_index + k < frame_samples;
           ++k) {
        const float t =
            static_cast<float>(k) / static_cast<float>(capture_config_.sample_rate_hz);
        const float envelope =
            1.0F - (static_cast<float>(k) / static_cast<float>(click_len));
        const float wave = std::sin(kTwoPi * frequency_hz * t);
        const float mixed =
            wave * envelope * accent_gain * metronome_gain * 15000.0F;
        for (std::size_t ch = 0; ch < channels; ++ch) {
          const std::size_t index = (sample_index + k) * channels + ch;
          if (index >= pcm.size()) {
            continue;
          }
          pcm[index] =
              ClampPcm(static_cast<float>(pcm[index]) + static_cast<float>(mixed));
        }
      }

      metronome_beat_index_ = (metronome_beat_index_ + 1U) % 4U;
    }
    metronome_phase_samples_ = (metronome_phase_samples_ + 1U) % samples_per_beat;
  }
}

}  // namespace tempolink::client
