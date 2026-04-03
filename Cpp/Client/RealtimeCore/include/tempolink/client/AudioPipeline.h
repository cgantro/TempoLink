#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <vector>

#include "tempolink/audio/IAudioInputDevice.h"
#include "tempolink/audio/IAudioOutputDevice.h"
#include "tempolink/client/AudioBridgePort.h"
#include "tempolink/client/audio/AudioDeviceManager.h"
#include "tempolink/client/audio/AudioProcessorChain.h"
#include "tempolink/client/audio/InputGainProcessor.h"
#include "tempolink/client/audio/LevelMeter.h"
#include "tempolink/client/audio/MetronomeProcessor.h"
#include "tempolink/client/audio/MultiStreamMixer.h"
#include "tempolink/client/audio/ReverbProcessor.h"

namespace tempolink::client {

/// Facade that orchestrates device management, processor chain,
/// level metering, and peer mixing. Each responsibility is delegated
/// to a focused component. The audio-thread callback uses a lock-free
/// atomic pointer swap instead of a mutex.
class AudioPipeline {
 public:
  using CapturedPcmCallback = std::function<void(std::span<const float>)>;

  bool Start(CapturedPcmCallback on_pcm_captured);
  void Stop();
  bool IsRunning() const;

  // --- Mute ---
  void SetMuted(bool muted);
  bool IsMuted() const;

  // --- Input Gain (delegated to InputGainProcessor) ---
  void SetInputGain(float gain);
  float InputGain() const;

  // --- Reverb (delegated to ReverbProcessor) ---
  void SetInputReverb(float amount);
  float InputReverb() const;

  // --- Master Volume ---
  void SetVolume(float volume);
  float Volume() const;

  // --- Level Metering (delegated to LevelMeter) ---
  float InputLevel() const;
  float OutputLevel() const;

  // --- Backend info ---
  std::string AudioBackendName() const;

  // --- Device Management (delegated to AudioDeviceManager) ---
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

  // --- Metronome (delegated to MetronomeProcessor) ---
  void SetMetronomeEnabled(bool enabled);
  bool IsMetronomeEnabled() const;
  void SetMetronomeBpm(int bpm);
  int MetronomeBpm() const;
  void SetMetronomeVolume(float volume);
  float MetronomeVolume() const;
  void SetMetronomeTone(int tone);
  int MetronomeTone() const;

  // --- Peer Mixing (delegated to MultiStreamMixer) ---
  void SetPeerMonitorVolume(std::uint32_t participant_id, float volume);
  float PeerMonitorVolume(std::uint32_t participant_id) const;
  void SetPeerMonitorPan(std::uint32_t participant_id, float pan);
  float PeerMonitorPan(std::uint32_t participant_id) const;

  void HandleIncomingAudio(std::uint32_t sender_participant_id, std::span<const float> pcm);

 private:
  void OnCapturedFrame(std::span<const float> pcm);
  void ResizeScratchBuffers();

  // Audio format configs
  tempolink::audio::AudioCaptureConfig capture_config_{};
  tempolink::audio::AudioPlaybackConfig playback_config_{};

  // Device management (extracted)
  audio::AudioDeviceManager device_manager_;
  std::shared_ptr<AudioBridgePort> audio_bridge_;
  std::atomic<AudioBridgePort*> audio_bridge_ptr_{nullptr};
  std::vector<std::shared_ptr<AudioBridgePort>> retired_audio_bridges_;

  // Processor chain (processes capture frames in order)
  audio::InputGainProcessor input_gain_processor_;
  audio::ReverbProcessor reverb_processor_;
  audio::MetronomeProcessor metronome_processor_;
  audio::MetronomeProcessor local_metronome_processor_;
  audio::AudioProcessorChain capture_chain_;

  // Peer mixing
  audio::MultiStreamMixer peer_mixer_;

  // Level meters
  audio::LevelMeter input_meter_;
  audio::LevelMeter output_meter_;

  // State
  std::atomic_bool running_{false};
  std::atomic_bool muted_{false};
  std::atomic<float> volume_{1.0F};

  /// Lock-free callback: audio thread reads via atomic load, main thread
  /// swaps via atomic exchange. shared_ptr is used so the old callback
  /// instance is destroyed only after the audio thread is done with it.
  using CallbackPtr = std::shared_ptr<CapturedPcmCallback>;
  std::atomic<CapturedPcmCallback*> active_callback_{nullptr};
  CallbackPtr callback_holder_;  // prevents premature destruction

  // Reused scratch buffers to avoid per-frame heap allocations on hot paths.
  std::vector<float> incoming_mix_buffer_;
  std::vector<std::int16_t> incoming_pcm16_buffer_;
  std::vector<float> capture_processed_buffer_;
  std::vector<float> capture_silence_buffer_;
  std::vector<std::int16_t> capture_pcm16_buffer_;
  std::vector<float> metronome_monitor_buffer_;
};

}  // namespace tempolink::client
