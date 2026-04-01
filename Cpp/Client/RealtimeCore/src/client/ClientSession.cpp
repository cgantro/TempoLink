#include "tempolink/client/ClientSession.h"

#include <algorithm>
#include <chrono>
#include <utility>

namespace tempolink::client {
namespace {

std::uint32_t ComputeRttMs(std::uint64_t sent_timestamp_us) {
  const auto now = std::chrono::steady_clock::now().time_since_epoch();
  const auto now_us = static_cast<std::uint64_t>(
      std::chrono::duration_cast<std::chrono::microseconds>(now).count());
  if (now_us < sent_timestamp_us) {
    return 0;
  }
  return static_cast<std::uint32_t>((now_us - sent_timestamp_us) / 1000ULL);
}

std::uint64_t NowMicros() {
  const auto now = std::chrono::steady_clock::now().time_since_epoch();
  return static_cast<std::uint64_t>(
      std::chrono::duration_cast<std::chrono::microseconds>(now).count());
}

}  // namespace

bool ClientSession::Start(const Config& config) {
  Stop();
  config_ = config;
  stats_ = {};
  joined_.store(false, std::memory_order_release);
  running_.store(false, std::memory_order_release);
  last_ping_sent_ = std::chrono::steady_clock::time_point::min();
  last_clock_sync_sent_ = std::chrono::steady_clock::time_point::min();
  peer_jitter_buffers_.clear();
  peer_levels_.clear();
  clock_sync_tracker_ = ClockSyncTracker{};

  ClientTransport::Endpoint endpoint;
  endpoint.server_host = config.server_host;
  endpoint.server_port = config.server_port;
  endpoint.room_id = config.room_id;
  endpoint.participant_id = config.participant_id;

  if (!transport_.Start(endpoint)) {
    return false;
  }

  const bool audio_started = audio_pipeline_.Start(
      [this](std::span<const std::byte> encoded_frame) {
        if (!running_.load(std::memory_order_acquire) ||
            !joined_.load(std::memory_order_acquire)) {
          return;
        }
        SendPacket(tempolink::net::PacketType::kAudio, encoded_frame);
      });
  if (!audio_started) {
    transport_.Stop();
    return false;
  }

  running_.store(true, std::memory_order_release);
  return true;
}

void ClientSession::Stop() {
  if (!running_.load(std::memory_order_acquire)) {
    return;
  }

  if (joined_.load(std::memory_order_acquire)) {
    SendPacket(tempolink::net::PacketType::kLeave, std::span<const std::byte>{});
  }

  joined_.store(false, std::memory_order_release);
  running_.store(false, std::memory_order_release);
  stats_.joined = false;
  stats_.connected = false;
  peer_jitter_buffers_.clear();
  peer_levels_.clear();
  audio_pipeline_.Stop();
  transport_.Stop();
}

bool ClientSession::Join() {
  if (!running_.load(std::memory_order_acquire)) {
    return false;
  }
  const bool ok = SendPacket(tempolink::net::PacketType::kJoin, config_.nickname);
  if (ok) {
    joined_.store(true, std::memory_order_release);
    stats_.joined = true;
  }
  return ok;
}

bool ClientSession::Leave() {
  if (!running_.load(std::memory_order_acquire) ||
      !joined_.load(std::memory_order_acquire)) {
    return false;
  }
  const bool ok =
      SendPacket(tempolink::net::PacketType::kLeave, std::span<const std::byte>{});
  joined_.store(false, std::memory_order_release);
  stats_.joined = false;
  return ok;
}

bool ClientSession::SendPing() {
  if (!running_.load(std::memory_order_acquire)) {
    return false;
  }
  last_ping_sent_ = std::chrono::steady_clock::now();
  return SendPacket(tempolink::net::PacketType::kPing, std::span<const std::byte>{});
}

bool ClientSession::Tick() {
  if (!running_.load(std::memory_order_acquire)) {
    return false;
  }

  for (auto it = peer_levels_.begin(); it != peer_levels_.end();) {
    it->second *= 0.90F;
    if (it->second < 0.01F) {
      it = peer_levels_.erase(it);
    } else {
      ++it;
    }
  }

  bool processed = false;
  tempolink::net::Packet packet;
  while (transport_.PollPacket(packet)) {
    processed = true;

    if (packet.header.type == tempolink::net::PacketType::kPong) {
      stats_.last_rtt_ms = ComputeRttMs(packet.header.timestamp_us);
      stats_.last_pong = std::chrono::steady_clock::now();
      stats_.connected = true;
      continue;
    }

    if (packet.header.type == tempolink::net::PacketType::kClockSyncAck) {
      if (clock_sync_tracker_.HandleAck(packet.payload, NowMicros())) {
        stats_.clock_offset_us = clock_sync_tracker_.ClockOffsetUs();
        stats_.last_rtt_ms = clock_sync_tracker_.RttMs();
      }
      continue;
    }

    if (packet.header.type == tempolink::net::PacketType::kAudio &&
        packet.header.sender_id != config_.participant_id) {
      auto& jitter_buffer = peer_jitter_buffers_[packet.header.sender_id];
      jitter_buffer.Push(packet.header.sequence, packet.header.timestamp_us,
                         packet.payload);
      auto& peer_level = peer_levels_[packet.header.sender_id];
      const float boost = std::clamp(
          static_cast<float>(packet.payload.size()) / 480.0F, 0.04F, 0.35F);
      peer_level = std::clamp(peer_level + boost, 0.0F, 1.0F);
    }
  }

  constexpr std::size_t kTargetJitterDepthPackets = 3;
  for (auto& [sender_id, jitter_buffer] : peer_jitter_buffers_) {
    (void)sender_id;
    const auto ready_frames = jitter_buffer.PopReady(kTargetJitterDepthPackets);
    for (const auto& frame : ready_frames) {
      audio_pipeline_.HandleIncomingAudio(frame.payload, sender_id);
    }
  }

  const auto now = std::chrono::steady_clock::now();
  if (stats_.last_pong != std::chrono::steady_clock::time_point::min()) {
    const auto since_pong =
        std::chrono::duration_cast<std::chrono::seconds>(now - stats_.last_pong);
    if (since_pong.count() > 3) {
      stats_.connected = false;
    }
  }

  static constexpr auto kPingInterval = std::chrono::seconds(1);
  static constexpr auto kClockSyncInterval = std::chrono::seconds(2);
  if (stats_.joined &&
      (last_ping_sent_ == std::chrono::steady_clock::time_point::min() ||
       now - last_ping_sent_ >= kPingInterval)) {
    SendPing();
  }
  if (stats_.joined &&
      (last_clock_sync_sent_ == std::chrono::steady_clock::time_point::min() ||
       now - last_clock_sync_sent_ >= kClockSyncInterval)) {
    SendClockSync();
  }

  return processed;
}

void ClientSession::SetMuted(bool muted) { audio_pipeline_.SetMuted(muted); }

bool ClientSession::IsMuted() const { return audio_pipeline_.IsMuted(); }

void ClientSession::SetInputGain(float gain) { audio_pipeline_.SetInputGain(gain); }

float ClientSession::InputGain() const { return audio_pipeline_.InputGain(); }

void ClientSession::SetInputReverb(float amount) {
  audio_pipeline_.SetInputReverb(amount);
}

float ClientSession::InputReverb() const { return audio_pipeline_.InputReverb(); }

void ClientSession::SetVolume(float volume) { audio_pipeline_.SetVolume(volume); }

float ClientSession::Volume() const { return audio_pipeline_.Volume(); }

float ClientSession::InputLevel() const { return audio_pipeline_.InputLevel(); }

float ClientSession::OutputLevel() const { return audio_pipeline_.OutputLevel(); }

float ClientSession::PeerLevel(std::uint32_t participant_id) const {
  const auto it = peer_levels_.find(participant_id);
  if (it == peer_levels_.end()) {
    return 0.0F;
  }
  return std::clamp(it->second, 0.0F, 1.0F);
}

void ClientSession::SetPeerMonitorVolume(std::uint32_t participant_id, float volume) {
  audio_pipeline_.SetPeerMonitorVolume(participant_id, volume);
}

void ClientSession::SetPeerMonitorPan(std::uint32_t participant_id, float pan) {
  audio_pipeline_.SetPeerMonitorPan(participant_id, pan);
}

float ClientSession::PeerMonitorVolume(std::uint32_t participant_id) const {
  return audio_pipeline_.PeerMonitorVolume(participant_id);
}

float ClientSession::PeerMonitorPan(std::uint32_t participant_id) const {
  return audio_pipeline_.PeerMonitorPan(participant_id);
}

std::string ClientSession::AudioBackendName() const {
  return audio_pipeline_.AudioBackendName();
}

std::vector<std::string> ClientSession::AvailableInputDevices() const {
  return audio_pipeline_.AvailableInputDevices();
}

std::vector<std::string> ClientSession::AvailableOutputDevices() const {
  return audio_pipeline_.AvailableOutputDevices();
}

bool ClientSession::SetInputDevice(const std::string& device_id) {
  return audio_pipeline_.SetInputDevice(device_id);
}

bool ClientSession::SetOutputDevice(const std::string& device_id) {
  return audio_pipeline_.SetOutputDevice(device_id);
}

std::string ClientSession::SelectedInputDevice() const {
  return audio_pipeline_.SelectedInputDevice();
}

std::string ClientSession::SelectedOutputDevice() const {
  return audio_pipeline_.SelectedOutputDevice();
}

bool ClientSession::ConfigureAudioFormat(std::uint32_t sample_rate_hz,
                                         std::uint16_t frame_samples) {
  return audio_pipeline_.ConfigureAudioFormat(sample_rate_hz, frame_samples);
}

std::uint32_t ClientSession::SampleRateHz() const {
  return audio_pipeline_.SampleRateHz();
}

std::uint16_t ClientSession::FrameSamples() const {
  return audio_pipeline_.FrameSamples();
}

void ClientSession::SetAudioBridge(std::shared_ptr<AudioBridgePort> audio_bridge) {
  audio_pipeline_.SetAudioBridge(std::move(audio_bridge));
}

void ClientSession::SetMetronomeEnabled(bool enabled) {
  audio_pipeline_.SetMetronomeEnabled(enabled);
}

bool ClientSession::IsMetronomeEnabled() const {
  return audio_pipeline_.IsMetronomeEnabled();
}

void ClientSession::SetMetronomeBpm(int bpm) { audio_pipeline_.SetMetronomeBpm(bpm); }

int ClientSession::MetronomeBpm() const { return audio_pipeline_.MetronomeBpm(); }

void ClientSession::SetMetronomeVolume(float volume) {
  audio_pipeline_.SetMetronomeVolume(volume);
}

float ClientSession::MetronomeVolume() const {
  return audio_pipeline_.MetronomeVolume();
}

const ClientSession::Stats& ClientSession::GetStats() const { return stats_; }

const ClientSession::Config& ClientSession::GetConfig() const { return config_; }

bool ClientSession::SendPacket(tempolink::net::PacketType type,
                               std::span<const std::byte> payload) {
  if (!running_.load(std::memory_order_acquire)) {
    return false;
  }
  return transport_.SendPacket(type, payload);
}

bool ClientSession::SendPacket(tempolink::net::PacketType type,
                               const std::string& text_payload) {
  if (!running_.load(std::memory_order_acquire)) {
    return false;
  }
  return transport_.SendTextPacket(type, text_payload);
}

bool ClientSession::SendClockSync() {
  if (!running_.load(std::memory_order_acquire)) {
    return false;
  }

  const auto now = std::chrono::steady_clock::now();
  const auto payload = clock_sync_tracker_.BuildRequestPayload(NowMicros());
  const bool ok = SendPacket(
      tempolink::net::PacketType::kClockSync,
      std::span<const std::byte>(payload.data(), payload.size()));
  if (ok) {
    last_clock_sync_sent_ = now;
  }
  return ok;
}

}  // namespace tempolink::client
