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
    if (!device.name.empty()) {
      ids.push_back(device.name);
    } else {
      ids.push_back(device.id);
    }
  }
  return ids;
}

std::string ToDisplayDevice(const std::vector<tempolink::audio::AudioDeviceInfo>& devices,
                            const std::string& token) {
  if (token.empty()) {
    for (const auto& device : devices) {
      if (device.is_default) {
        return device.name.empty() ? device.id : device.name;
      }
    }
    if (!devices.empty()) {
      return devices.front().name.empty() ? devices.front().id : devices.front().name;
    }
    return {};
  }

  for (const auto& device : devices) {
    if (device.id == token || device.name == token) {
      return device.name.empty() ? device.id : device.name;
    }
  }
  return token;
}

std::int16_t ClampPcm(float sample) {
  constexpr float kMin = -32768.0F;
  constexpr float kMax = 32767.0F;
  const float clamped = std::clamp(sample, kMin, kMax);
  return static_cast<std::int16_t>(clamped);
}

float ComputeNormalizedPeak(std::span<const std::int16_t> pcm) {
  if (pcm.empty()) {
    return 0.0F;
  }

  std::int32_t max_abs = 0;
  for (const auto sample : pcm) {
    const auto abs_sample = std::abs(static_cast<std::int32_t>(sample));
    if (abs_sample > max_abs) {
      max_abs = abs_sample;
    }
  }

  constexpr float kInvMax = 1.0F / 32768.0F;
  return std::clamp(static_cast<float>(max_abs) * kInvMax, 0.0F, 1.0F);
}

float SmoothLevel(float previous, float incoming) {
  // Fast rise, slower fall to make meter readable without heavy jitter.
  constexpr float kRise = 0.45F;
  constexpr float kFall = 0.12F;
  const float alpha = incoming >= previous ? kRise : kFall;
  return std::clamp(previous + (incoming - previous) * alpha, 0.0F, 1.0F);
}

void ComputePanGains(float pan, float& left_gain, float& right_gain) {
  constexpr float kPiOverFour = 0.78539816339F;
  const float clamped_pan = std::clamp(pan, -1.0F, 1.0F);
  const float angle = (clamped_pan + 1.0F) * kPiOverFour;
  left_gain = std::cos(angle);
  right_gain = std::sin(angle);
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
  reverb_write_index_ = 0;
  {
    std::scoped_lock lock(peer_mix_mutex_);
    peer_monitor_mix_.clear();
  }
  const std::size_t reverb_len =
      static_cast<std::size_t>(capture_config_.sample_rate_hz / 20U) *
      std::max<std::size_t>(1, capture_config_.channels);
  reverb_delay_line_.assign(std::max<std::size_t>(1, reverb_len), 0);

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
  reverb_delay_line_.clear();
  reverb_write_index_ = 0;
  {
    std::scoped_lock lock(peer_mix_mutex_);
    peer_monitor_mix_.clear();
  }
}

bool AudioPipeline::IsRunning() const { return running_.load(); }

void AudioPipeline::SetMuted(bool muted) { muted_.store(muted); }

bool AudioPipeline::IsMuted() const { return muted_.load(); }

void AudioPipeline::SetInputGain(float gain) {
  input_gain_.store(std::clamp(gain, 0.0F, 2.0F));
}

float AudioPipeline::InputGain() const { return input_gain_.load(); }

void AudioPipeline::SetInputReverb(float amount) {
  input_reverb_.store(std::clamp(amount, 0.0F, 1.0F));
}

float AudioPipeline::InputReverb() const { return input_reverb_.load(); }

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
  auto temp_input = tempolink::audio::CreateDefaultAudioInputDevice();
  if (!temp_input) {
    return selected_input_device_;
  }
  return ToDisplayDevice(temp_input->ListDevices(), selected_input_device_);
}

std::string AudioPipeline::SelectedOutputDevice() const {
  if (audio_output_) {
    return ToDisplayDevice(audio_output_->ListDevices(), audio_output_->SelectedDeviceId());
  }
  auto temp_output = tempolink::audio::CreateDefaultAudioOutputDevice();
  if (!temp_output) {
    return selected_output_device_;
  }
  return ToDisplayDevice(temp_output->ListDevices(), selected_output_device_);
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
  reverb_write_index_ = 0;
  const std::size_t reverb_len =
      static_cast<std::size_t>(capture_config_.sample_rate_hz / 20U) *
      std::max<std::size_t>(1, capture_config_.channels);
  reverb_delay_line_.assign(std::max<std::size_t>(1, reverb_len), 0);
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

void AudioPipeline::SetPeerMonitorVolume(std::uint32_t participant_id, float volume) {
  if (participant_id == 0) {
    return;
  }
  std::scoped_lock lock(peer_mix_mutex_);
  auto& mix = peer_monitor_mix_[participant_id];
  mix.volume = std::clamp(volume, 0.0F, 1.5F);
}

void AudioPipeline::SetPeerMonitorPan(std::uint32_t participant_id, float pan) {
  if (participant_id == 0) {
    return;
  }
  std::scoped_lock lock(peer_mix_mutex_);
  auto& mix = peer_monitor_mix_[participant_id];
  mix.pan = std::clamp(pan, -1.0F, 1.0F);
}

float AudioPipeline::PeerMonitorVolume(std::uint32_t participant_id) const {
  if (participant_id == 0) {
    return 1.0F;
  }
  std::scoped_lock lock(peer_mix_mutex_);
  const auto it = peer_monitor_mix_.find(participant_id);
  if (it == peer_monitor_mix_.end()) {
    return 1.0F;
  }
  return it->second.volume;
}

float AudioPipeline::PeerMonitorPan(std::uint32_t participant_id) const {
  if (participant_id == 0) {
    return 0.0F;
  }
  std::scoped_lock lock(peer_mix_mutex_);
  const auto it = peer_monitor_mix_.find(participant_id);
  if (it == peer_monitor_mix_.end()) {
    return 0.0F;
  }
  return it->second.pan;
}

void AudioPipeline::HandleIncomingAudio(
    std::span<const std::byte> encoded_payload, std::uint32_t sender_participant_id) {
  if (!running_.load() || !audio_decoder_ || !audio_output_) {
    return;
  }

  auto decoded = audio_decoder_->Decode(encoded_payload);
  if (decoded.empty()) {
    return;
  }

  const float decoded_peak = ComputeNormalizedPeak(
      std::span<const std::int16_t>(decoded.data(), decoded.size()));
  const float prev_output_level = output_level_.load();
  output_level_.store(SmoothLevel(prev_output_level, decoded_peak));

  PeerMonitorMix peer_mix;
  {
    std::scoped_lock lock(peer_mix_mutex_);
    const auto it = peer_monitor_mix_.find(sender_participant_id);
    if (it != peer_monitor_mix_.end()) {
      peer_mix = it->second;
    }
  }

  const float global_gain = volume_.load();
  const float peer_gain = std::clamp(peer_mix.volume, 0.0F, 1.5F);
  const float total_gain = global_gain * peer_gain;
  const std::size_t channels = std::max<std::size_t>(1, playback_config_.channels);
  if (channels >= 2 && decoded.size() >= 2) {
    float left_pan_gain = 1.0F;
    float right_pan_gain = 1.0F;
    ComputePanGains(peer_mix.pan, left_pan_gain, right_pan_gain);
    for (std::size_t i = 0; i < decoded.size(); i += channels) {
      if (i >= decoded.size()) {
        break;
      }
      decoded[i] = ClampPcm(static_cast<float>(decoded[i]) * total_gain * left_pan_gain);
      if (i + 1U < decoded.size()) {
        decoded[i + 1] =
            ClampPcm(static_cast<float>(decoded[i + 1]) * total_gain * right_pan_gain);
      }
      for (std::size_t ch = 2; ch < channels && i + ch < decoded.size(); ++ch) {
        decoded[i + ch] =
            ClampPcm(static_cast<float>(decoded[i + ch]) * total_gain);
      }
    }
  } else {
    for (auto& sample : decoded) {
      sample = ClampPcm(static_cast<float>(sample) * total_gain);
    }
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
  if (!running_.load() || !audio_encoder_) {
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

  std::vector<std::int16_t> mixed(pcm.begin(), pcm.begin() + expected_samples);
  const float input_gain = input_gain_.load();
  if (std::abs(input_gain - 1.0F) > 0.0001F) {
    for (auto& sample : mixed) {
      sample = ClampPcm(static_cast<float>(sample) * input_gain);
    }
  }
  const float input_peak = ComputeNormalizedPeak(
      std::span<const std::int16_t>(mixed.data(), mixed.size()));
  const float prev_input_level = input_level_.load();
  input_level_.store(SmoothLevel(prev_input_level, input_peak));
  ApplySimpleReverb(mixed);
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

void AudioPipeline::ApplySimpleReverb(std::vector<std::int16_t>& pcm) {
  const float amount = input_reverb_.load();
  if (amount <= 0.0001F || pcm.empty() || reverb_delay_line_.empty()) {
    return;
  }

  const float wet = std::clamp(amount, 0.0F, 1.0F) * 0.55F;
  const float feedback = 0.30F + (wet * 0.35F);
  for (auto& sample : pcm) {
    const float dry_sample = static_cast<float>(sample);
    const float delayed = static_cast<float>(reverb_delay_line_[reverb_write_index_]);
    const float mixed = dry_sample + (delayed * wet);
    sample = ClampPcm(mixed);
    const float next_delay = (dry_sample * 0.45F) + (delayed * feedback);
    reverb_delay_line_[reverb_write_index_] = ClampPcm(next_delay);
    ++reverb_write_index_;
    if (reverb_write_index_ >= reverb_delay_line_.size()) {
      reverb_write_index_ = 0;
    }
  }
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
