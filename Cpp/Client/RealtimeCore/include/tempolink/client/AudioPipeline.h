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

#include "tempolink/audio/IAudioInputDevice.h"
#include "tempolink/audio/IAudioOutputDevice.h"
#include "tempolink/client/AudioBridgePort.h"
#include "tempolink/client/audio/MetronomeProcessor.h"
#include "tempolink/client/audio/ReverbProcessor.h"
#include "tempolink/client/audio/MultiStreamMixer.h"

namespace tempolink::client {

class AudioPipeline {
 public:
  using CapturedPcmCallback = std::function<void(std::span<const float>)>;
  
  bool Start(CapturedPcmCallback on_pcm_captured);
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
  float PeerMonitorVolume(std::uint32_t participant_id) const;
  void SetPeerMonitorPan(std::uint32_t participant_id, float pan);
  float PeerMonitorPan(std::uint32_t participant_id) const;

  void HandleIncomingAudio(std::uint32_t sender_participant_id, std::span<const float> pcm);

 private:
  void OnCapturedFrame(std::span<const float> pcm);

  tempolink::audio::AudioCaptureConfig capture_config_{};
  tempolink::audio::AudioPlaybackConfig playback_config_{};
  std::unique_ptr<tempolink::audio::IAudioInputDevice> audio_input_;
  std::unique_ptr<tempolink::audio::IAudioOutputDevice> audio_output_;
  std::shared_ptr<AudioBridgePort> audio_bridge_;

  audio::MetronomeProcessor metronome_processor_;
  audio::ReverbProcessor reverb_processor_;
  audio::MultiStreamMixer peer_mixer_;

  mutable std::mutex callback_mutex_;
  CapturedPcmCallback captured_pcm_callback_;
  std::atomic_bool running_{false};
  std::atomic_bool muted_{false};
  std::atomic<float> input_gain_{1.0F};
  std::atomic<float> volume_{1.0F};
  std::atomic<float> input_level_{0.0F};
  std::atomic<float> output_level_{0.0F};
  std::string selected_input_device_;
  std::string selected_output_device_;
};

}  // namespace tempolink::client
