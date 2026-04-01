#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <atomic>
#include <unordered_map>
#include <span>
#include <string>
#include <vector>
#include <memory>

#include "tempolink/client/AudioPipeline.h"
#include "tempolink/client/ClientTransport.h"
#include "tempolink/client/ClockSyncTracker.h"
#include "tempolink/client/PeerJitterBuffer.h"
#include "tempolink/config/NetworkConstants.h"

namespace tempolink::client {

class ClientSession {
 public:
  struct Config {
    std::string server_host = tempolink::config::kDefaultRelayHost;
    std::uint16_t server_port = tempolink::config::kDefaultRelayPort;
    std::uint32_t room_id = tempolink::config::kDefaultRoomId;
    std::uint32_t participant_id = tempolink::config::kDefaultParticipantId;
    std::string nickname = tempolink::config::kDefaultNickname;
  };

  struct Stats {
    bool joined = false;
    bool connected = false;
    std::uint32_t last_rtt_ms = 0;
    std::int64_t clock_offset_us = 0;
    std::chrono::steady_clock::time_point last_pong =
        std::chrono::steady_clock::time_point::min();
  };

  bool Start(const Config& config);
  void Stop();

  bool Join();
  bool Leave();
  bool SendPing();
  bool Tick();

  void SetMuted(bool muted);
  bool IsMuted() const;
  void SetVolume(float volume);
  float Volume() const;
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

  const Stats& GetStats() const;
  const Config& GetConfig() const;

 private:
  bool SendPacket(tempolink::net::PacketType type, std::span<const std::byte> payload);
  bool SendPacket(tempolink::net::PacketType type, const std::string& text_payload);
  bool SendClockSync();

  Config config_;
  Stats stats_;
  std::atomic_bool running_{false};
  std::atomic_bool joined_{false};
  std::chrono::steady_clock::time_point last_ping_sent_ =
      std::chrono::steady_clock::time_point::min();
  std::chrono::steady_clock::time_point last_clock_sync_sent_ =
      std::chrono::steady_clock::time_point::min();
  ClientTransport transport_;
  AudioPipeline audio_pipeline_;
  ClockSyncTracker clock_sync_tracker_;
  std::unordered_map<std::uint32_t, PeerJitterBuffer> peer_jitter_buffers_;
};

}  // namespace tempolink::client
