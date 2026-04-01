#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <memory>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

#include "tempolink/audio/IAudioCodec.h"
#include "tempolink/audio/IAudioInputDevice.h"
#include "tempolink/audio/IAudioOutputDevice.h"
#include "tempolink/client/AudioBridgePort.h"

namespace tempolink::client {

class AudioPipeline {
 public:
  using EncodedFrameCallback = std::function<void(std::span<const std::byte>)>;

  bool Start(EncodedFrameCallback on_frame_encoded);
  void Stop();
  bool IsRunning() const;

  void SetMuted(bool muted);
  bool IsMuted() const;
  void SetInputGain(float gain);
  float InputGain() const;
  void SetInputReverb(float amount);
  float InputReverb() const;
  void SetVolume(float volume);
  float Volume() const;
  float InputLevel() const;
  float OutputLevel() const;
  std::string AudioBackendName() const;

  std::vector<std::string> AvailableInputDevices() const;
  std::vector<std::string> AvailableOutputDevices() const;
  bool SetInputDevice(const std::string& device_id);
  bool SetOutputDevice(const std::string& device_id);
  std::string SelectedInputDevice() const;
  std::string SelectedOutputDevice() const;
  bool ConfigureAudioFormat(std::uint32_t sample_rate_hz, std::uint16_t frame_samples);
  std::uint32_t SampleRateHz() const;
  std::uint16_t FrameSamples() const;
  void SetAudioBridge(std::shared_ptr<AudioBridgePort> audio_bridge);

  void SetMetronomeEnabled(bool enabled);
  bool IsMetronomeEnabled() const;
  void SetMetronomeBpm(int bpm);
  int MetronomeBpm() const;
  void SetMetronomeVolume(float volume);
  float MetronomeVolume() const;

  void SetPeerMonitorVolume(std::uint32_t participant_id, float volume);
  void SetPeerMonitorPan(std::uint32_t participant_id, float pan);
  float PeerMonitorVolume(std::uint32_t participant_id) const;
  float PeerMonitorPan(std::uint32_t participant_id) const;

  void HandleIncomingAudio(std::span<const std::byte> encoded_payload,
                           std::uint32_t sender_participant_id = 0);

 private:
  void OnCapturedFrame(std::span<const std::int16_t> pcm);
  void ApplySimpleReverb(std::vector<std::int16_t>& pcm);
  void MixMetronomeClick(std::vector<std::int16_t>& pcm);

  struct PeerMonitorMix {
    float volume = 1.0F;
    float pan = 0.0F;
  };

  tempolink::audio::AudioCaptureConfig capture_config_{};
  tempolink::audio::AudioPlaybackConfig playback_config_{};
  std::unique_ptr<tempolink::audio::IAudioInputDevice> audio_input_;
  std::unique_ptr<tempolink::audio::IAudioOutputDevice> audio_output_;
  std::unique_ptr<tempolink::audio::IAudioCodec> audio_encoder_;
  std::unique_ptr<tempolink::audio::IAudioCodec> audio_decoder_;
  std::shared_ptr<AudioBridgePort> audio_bridge_;

  mutable std::mutex callback_mutex_;
  EncodedFrameCallback encoded_frame_callback_;
  std::atomic_bool running_{false};
  std::atomic_bool muted_{false};
  std::atomic<float> input_gain_{1.0F};
  std::atomic<float> input_reverb_{0.0F};
  std::atomic<float> volume_{1.0F};
  std::atomic<float> input_level_{0.0F};
  std::atomic<float> output_level_{0.0F};
  std::string selected_input_device_;
  std::string selected_output_device_;

  std::atomic_bool metronome_enabled_{false};
  std::atomic_int metronome_bpm_{120};
  std::atomic<float> metronome_volume_{0.35F};
  std::uint64_t metronome_phase_samples_ = 0;
  std::uint32_t metronome_beat_index_ = 0;
  std::vector<std::int16_t> reverb_delay_line_;
  std::size_t reverb_write_index_ = 0;
  mutable std::mutex peer_mix_mutex_;
  std::unordered_map<std::uint32_t, PeerMonitorMix> peer_monitor_mix_;
};

}  // namespace tempolink::client
