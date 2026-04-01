#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <span>
#include <string>
#include <vector>

#include "tempolink/bridge/UdpBridgeProtocol.h"
#include "tempolink/client/AudioBridgePort.h"
#include "tempolink/net/UdpSocket.h"

namespace tempolink::juceapp::bridge {

class UdpAudioBridgePort final : public tempolink::client::AudioBridgePort {
 public:
  UdpAudioBridgePort(std::string host, std::uint16_t client_listen_port,
                     std::uint16_t plugin_listen_port);
  ~UdpAudioBridgePort() override;

  void OnCapturedInput(std::span<std::int16_t> frame,
                       const tempolink::audio::AudioCaptureConfig& config) override;

  void OnPlaybackOutput(std::span<const std::int16_t> frame,
                        const tempolink::audio::AudioPlaybackConfig& config) override;

 private:
  bool EnsureSocketsReady();
  bool TryReadDawFrame(std::size_t expected_samples, std::uint32_t sample_rate_hz,
                       std::uint16_t channels, std::vector<std::int16_t>& out);
  bool SendToPlugin(tempolink::bridge::UdpBridgePacketType type,
                    std::span<const std::int16_t> samples, std::uint32_t sample_rate_hz,
                    std::uint16_t channels, std::uint16_t frames);

  std::mutex io_mutex_;
  tempolink::net::UdpSocket rx_socket_;
  tempolink::net::UdpSocket tx_socket_;
  std::string host_;
  std::uint16_t client_listen_port_ = 0;
  std::uint16_t plugin_listen_port_ = 0;
  std::atomic_bool sockets_ready_{false};
  std::uint64_t tx_sequence_ = 0;
};

}  // namespace tempolink::juceapp::bridge
