#include "tempolink/client/AudioPipeline.h"

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <vector>

#include "tempolink/config/AudioConstants.h"

namespace tempolink::client {
namespace {

std::int16_t FloatToPcm16(float sample) {
  const float clamped = std::clamp(sample, -1.0F, 1.0F);
  return static_cast<std::int16_t>(clamped * 32767.0F);
}

float Pcm16ToFloat(std::int16_t sample) {
  return static_cast<float>(sample) / 32767.0F;
}

}  // namespace

// -----------------------------------------------------------------------
// Start / Stop
// -----------------------------------------------------------------------

bool AudioPipeline::Start(CapturedPcmCallback on_pcm_captured) {
  Stop();

  if (!device_manager_.CreateDevices()) {
    return false;
  }

  if (!device_manager_.StartOutput(playback_config_)) {
    Stop();
    return false;
  }
  device_manager_.SetOutputVolume(volume_.load());

  // Install callback via atomic swap — no mutex on the audio thread.
  callback_holder_ = std::make_shared<CapturedPcmCallback>(std::move(on_pcm_captured));
  active_callback_.store(callback_holder_.get(), std::memory_order_release);

  running_ = true;
  ResizeScratchBuffers();

  // Build the capture processor chain
  capture_chain_.Clear();
  capture_chain_.AddProcessor(&input_gain_processor_);
  capture_chain_.AddProcessor(&reverb_processor_);
  capture_chain_.AddProcessor(&metronome_processor_);

  reverb_processor_.Reset();
  input_meter_.Reset();
  output_meter_.Reset();

  if (!device_manager_.StartInput(
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
  device_manager_.StopInput();
  device_manager_.StopOutput();
  device_manager_.DestroyDevices();

  // Clear callback atomically
  active_callback_.store(nullptr, std::memory_order_release);
  callback_holder_.reset();

  capture_chain_.Clear();
  audio_bridge_ptr_.store(nullptr, std::memory_order_release);
  retired_audio_bridges_.clear();
}

bool AudioPipeline::IsRunning() const { return running_.load(); }

// -----------------------------------------------------------------------
// Mute
// -----------------------------------------------------------------------

void AudioPipeline::SetMuted(bool muted) { muted_.store(muted); }
bool AudioPipeline::IsMuted() const { return muted_.load(); }

// -----------------------------------------------------------------------
// Input Gain (delegated)
// -----------------------------------------------------------------------

void AudioPipeline::SetInputGain(float gain) { input_gain_processor_.SetGain(gain); }
float AudioPipeline::InputGain() const { return input_gain_processor_.Gain(); }

// -----------------------------------------------------------------------
// Reverb (delegated)
// -----------------------------------------------------------------------

void AudioPipeline::SetInputReverb(float amount) { reverb_processor_.SetAmount(amount); }
float AudioPipeline::InputReverb() const { return reverb_processor_.Amount(); }

// -----------------------------------------------------------------------
// Volume
// -----------------------------------------------------------------------

void AudioPipeline::SetVolume(float volume) {
  const float clamped = std::clamp(volume, 0.0F, 1.0F);
  volume_.store(clamped);
  device_manager_.SetOutputVolume(clamped);
}
float AudioPipeline::Volume() const { return volume_.load(); }

// -----------------------------------------------------------------------
// Level Metering (delegated)
// -----------------------------------------------------------------------

float AudioPipeline::InputLevel() const { return input_meter_.Level(); }
float AudioPipeline::OutputLevel() const { return output_meter_.Level(); }

// -----------------------------------------------------------------------
// Backend
// -----------------------------------------------------------------------

std::string AudioPipeline::AudioBackendName() const {
  return device_manager_.InputBackendName() + " / " +
         device_manager_.OutputBackendName();
}

// -----------------------------------------------------------------------
// Device Management (delegated to AudioDeviceManager)
// -----------------------------------------------------------------------

std::vector<std::string> AudioPipeline::AvailableInputDevices() const {
  return device_manager_.AvailableInputDevices();
}

std::vector<std::string> AudioPipeline::AvailableOutputDevices() const {
  return device_manager_.AvailableOutputDevices();
}

bool AudioPipeline::SetInputDevice(const std::string& device_id) {
  return device_manager_.SelectInputDevice(device_id);
}

bool AudioPipeline::SetOutputDevice(const std::string& device_id) {
  return device_manager_.SelectOutputDevice(device_id);
}

std::string AudioPipeline::SelectedInputDevice() const {
  return device_manager_.SelectedInputDevice();
}

std::string AudioPipeline::SelectedOutputDevice() const {
  return device_manager_.SelectedOutputDevice();
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
  ResizeScratchBuffers();

  if (!running_.load()) {
    return true;
  }

  // Hot-reconfigure: restart devices with new format
  device_manager_.StopInput();
  device_manager_.StopOutput();

  if (!device_manager_.StartOutput(playback_config_)) {
    Stop();
    return false;
  }
  device_manager_.SetOutputVolume(volume_.load());
  if (!device_manager_.StartInput(
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
  if (running_.load(std::memory_order_acquire) && audio_bridge_ &&
      audio_bridge_.get() != audio_bridge.get()) {
    retired_audio_bridges_.push_back(audio_bridge_);
  }
  audio_bridge_ = std::move(audio_bridge);
  audio_bridge_ptr_.store(audio_bridge_ ? audio_bridge_.get() : nullptr,
                          std::memory_order_release);
  if (!running_.load(std::memory_order_acquire)) {
    retired_audio_bridges_.clear();
  }
}

// -----------------------------------------------------------------------
// Metronome (delegated)
// -----------------------------------------------------------------------

void AudioPipeline::SetMetronomeEnabled(bool enabled) {
  metronome_processor_.SetEnabled(enabled);
  local_metronome_processor_.SetEnabled(enabled);
}
bool AudioPipeline::IsMetronomeEnabled() const { return metronome_processor_.IsEnabled(); }
void AudioPipeline::SetMetronomeBpm(int bpm) {
  metronome_processor_.SetBpm(bpm);
  local_metronome_processor_.SetBpm(bpm);
}
int AudioPipeline::MetronomeBpm() const { return metronome_processor_.Bpm(); }
void AudioPipeline::SetMetronomeVolume(float volume) {
  metronome_processor_.SetVolume(volume);
  local_metronome_processor_.SetVolume(volume);
}
float AudioPipeline::MetronomeVolume() const { return metronome_processor_.Volume(); }
void AudioPipeline::SetMetronomeTone(int tone) {
  metronome_processor_.SetTone(tone);
  local_metronome_processor_.SetTone(tone);
}
int AudioPipeline::MetronomeTone() const { return metronome_processor_.Tone(); }

// -----------------------------------------------------------------------
// Peer Mixing (delegated)
// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------
// Incoming peer audio → mix → output
// -----------------------------------------------------------------------

void AudioPipeline::HandleIncomingAudio(std::uint32_t sender_participant_id,
                                        std::span<const float> pcm) {
  if (!running_.load() || !device_manager_.HasOutput()) {
    return;
  }
  const std::size_t channels = std::max<std::size_t>(1, playback_config_.channels);
  const std::size_t expected_samples =
      static_cast<std::size_t>(playback_config_.frame_samples) * channels;
  const std::size_t mix_samples = std::min(pcm.size(), expected_samples);
  if (mix_samples == 0) {
    return;
  }
  if (incoming_mix_buffer_.size() < mix_samples ||
      incoming_pcm16_buffer_.size() < mix_samples) {
    // Non-audio callback path fallback: keep correctness if format changed unexpectedly.
    incoming_mix_buffer_.resize(mix_samples, 0.0F);
    incoming_pcm16_buffer_.resize(mix_samples, 0);
  }
  std::fill_n(incoming_mix_buffer_.begin(), mix_samples, 0.0F);

  audio::AudioFormat format;
  format.sample_rate_hz = playback_config_.sample_rate_hz;
  format.channels = playback_config_.channels;
  format.frame_samples = playback_config_.frame_samples;

  peer_mixer_.MixPeer(std::span<float>(incoming_mix_buffer_.data(), mix_samples),
                      pcm.subspan(0, mix_samples), sender_participant_id, format);

  auto* bridge = audio_bridge_ptr_.load(std::memory_order_acquire);
  if (bridge) {
    for (std::size_t i = 0; i < mix_samples; ++i) {
      incoming_pcm16_buffer_[i] = FloatToPcm16(incoming_mix_buffer_[i]);
    }
    bridge->OnPlaybackOutput(
        std::span<const std::int16_t>(incoming_pcm16_buffer_.data(), mix_samples),
        playback_config_);
  }

  output_meter_.Update(std::span<const float>(incoming_mix_buffer_.data(), mix_samples));
  device_manager_.PlayFrame(std::span<const float>(incoming_mix_buffer_.data(), mix_samples));
}

// -----------------------------------------------------------------------
// Capture frame callback (runs on audio thread — lock-free)
// -----------------------------------------------------------------------

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

  if (capture_processed_buffer_.size() < expected_samples ||
      capture_pcm16_buffer_.size() < expected_samples ||
      metronome_monitor_buffer_.size() < expected_samples) {
    return;
  }
  const bool muted = muted_.load();
  if (muted) {
    std::fill_n(capture_processed_buffer_.begin(), expected_samples, 0.0F);
  } else {
    std::copy_n(pcm.begin(), expected_samples, capture_processed_buffer_.begin());
  }

  // Run through the processor chain: InputGain → Reverb → Metronome
  audio::AudioFormat format;
  format.sample_rate_hz = capture_config_.sample_rate_hz;
  format.channels = capture_config_.channels;
  format.frame_samples = capture_config_.frame_samples;
  auto processed =
      std::span<float>(capture_processed_buffer_.data(), expected_samples);
  capture_chain_.Process(processed, format);

  auto* bridge = audio_bridge_ptr_.load(std::memory_order_acquire);
  if (bridge) {
    for (std::size_t i = 0; i < expected_samples; ++i) {
      capture_pcm16_buffer_[i] = FloatToPcm16(processed[i]);
    }
    bridge->OnCapturedInput(
        std::span<std::int16_t>(capture_pcm16_buffer_.data(), expected_samples),
        capture_config_);
    for (std::size_t i = 0; i < expected_samples; ++i) {
      processed[i] = Pcm16ToFloat(capture_pcm16_buffer_[i]);
    }
  }

  input_meter_.Update(processed);

  if (device_manager_.HasOutput() && local_metronome_processor_.IsEnabled()) {
    std::fill_n(metronome_monitor_buffer_.begin(), expected_samples, 0.0F);
    auto monitor =
        std::span<float>(metronome_monitor_buffer_.data(), expected_samples);
    local_metronome_processor_.Process(monitor, format);
    bool has_click = false;
    for (float sample : monitor) {
      if (std::abs(sample) > 1.0e-4F) {
        has_click = true;
        break;
      }
    }
    if (has_click) {
      device_manager_.PlayFrame(monitor);
    }
  }

  // Lock-free callback dispatch: atomic load, no mutex
  auto* cb = active_callback_.load(std::memory_order_acquire);
  if (cb && *cb) {
    (*cb)(processed);
  }
}

void AudioPipeline::ResizeScratchBuffers() {
  const std::size_t capture_channels = std::max<std::size_t>(1, capture_config_.channels);
  const std::size_t playback_channels = std::max<std::size_t>(1, playback_config_.channels);
  const std::size_t capture_samples =
      static_cast<std::size_t>(capture_config_.frame_samples) * capture_channels;
  const std::size_t playback_samples =
      static_cast<std::size_t>(playback_config_.frame_samples) * playback_channels;
  capture_processed_buffer_.assign(capture_samples, 0.0F);
  capture_silence_buffer_.assign(capture_samples, 0.0F);
  capture_pcm16_buffer_.assign(capture_samples, 0);
  metronome_monitor_buffer_.assign(capture_samples, 0.0F);
  incoming_mix_buffer_.assign(playback_samples, 0.0F);
  incoming_pcm16_buffer_.assign(playback_samples, 0);
}

}  // namespace tempolink::client
