#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <span>
#include <string>
#include <vector>

#include "tempolink/client/AudioPipeline.h"
#include "tempolink/client/ClientTransport.h"
#include "tempolink/client/ClockSyncTracker.h"
#include "tempolink/client/PeerJitterBuffer.h"

namespace tempolink::client {

class ClientSession {
 public:
  struct Config {
    std::string server_host = "127.0.0.1";
    std::uint16_t server_port = 40000;
    std::uint32_t room_id = 1;
    std::uint32_t participant_id = 1001;
    std::string nickname = "guest";
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
  bool running_ = false;
  bool joined_ = false;
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
