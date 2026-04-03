#include "tempolink/juce/bridge/UdpAudioBridgePort.h"

#include <algorithm>
#include <utility>

namespace tempolink::juceapp::bridge {
namespace {

std::size_t ExpectedSamples(std::uint16_t frames, std::uint8_t channels) {
  const auto safe_channels =
      static_cast<std::size_t>(std::max<std::uint8_t>(1U, channels));
  return static_cast<std::size_t>(frames) * safe_channels;
}

}  // namespace

UdpAudioBridgePort::UdpAudioBridgePort(std::string host,
                                       std::uint16_t client_listen_port,
                                       std::uint16_t plugin_listen_port)
    : host_(std::move(host)),
      client_listen_port_(client_listen_port),
      plugin_listen_port_(plugin_listen_port) {}

UdpAudioBridgePort::~UdpAudioBridgePort() {
  std::scoped_lock lock(io_mutex_);
  rx_socket_.Close();
  tx_socket_.Close();
  sockets_ready_.store(false, std::memory_order_release);
}

void UdpAudioBridgePort::OnCapturedInput(
    std::span<std::int16_t> frame,
    const tempolink::audio::AudioCaptureConfig& config) {
  if (frame.empty()) {
    return;
  }
  if (!EnsureSocketsReady()) {
    return;
  }

  const std::size_t expected_samples =
      ExpectedSamples(config.frame_samples, config.channels);
  if (expected_samples == 0U) {
    return;
  }

  std::vector<std::int16_t> daw_frame;
  if (!TryReadDawFrame(expected_samples, config.sample_rate_hz, config.channels,
                       daw_frame)) {
    return;
  }

  const std::size_t copy_count = std::min(frame.size(), daw_frame.size());
  if (copy_count == 0U) {
    return;
  }
  std::copy_n(daw_frame.begin(), copy_count, frame.begin());
}

void UdpAudioBridgePort::OnPlaybackOutput(
    std::span<const std::int16_t> frame,
    const tempolink::audio::AudioPlaybackConfig& config) {
  if (frame.empty()) {
    return;
  }
  const auto frames = static_cast<std::uint16_t>(
      std::min<std::size_t>(config.frame_samples, 0xFFFFU));
  SendToPlugin(tempolink::bridge::UdpBridgePacketType::kClientToDaw, frame,
               config.sample_rate_hz, config.channels, frames);
}

bool UdpAudioBridgePort::EnsureSocketsReady() {
  if (sockets_ready_.load(std::memory_order_acquire)) {
    return true;
  }

  std::scoped_lock lock(io_mutex_);
  if (sockets_ready_.load(std::memory_order_relaxed)) {
    return true;
  }

  if (!rx_socket_.Open()) {
    return false;
  }
  if (!rx_socket_.Bind(client_listen_port_)) {
    rx_socket_.Close();
    return false;
  }
  if (!rx_socket_.SetNonBlocking(true)) {
    rx_socket_.Close();
    return false;
  }

  if (!tx_socket_.Open()) {
    rx_socket_.Close();
    return false;
  }
  if (!tx_socket_.SetNonBlocking(true)) {
    tx_socket_.Close();
    rx_socket_.Close();
    return false;
  }

  sockets_ready_.store(true, std::memory_order_release);
  return true;
}

bool UdpAudioBridgePort::TryReadDawFrame(std::size_t expected_samples,
                                         std::uint32_t sample_rate_hz,
                                         std::uint16_t channels,
                                         std::vector<std::int16_t>& out) {
  (void)channels;
  const std::size_t max_datagram_bytes =
      expected_samples * sizeof(std::int16_t) + 64U;
  const std::size_t read_capacity =
      std::min(max_datagram_bytes, rx_datagram_buffer_.size());
  if (read_capacity == 0U) {
    return false;
  }

  std::scoped_lock lock(io_mutex_);
  const auto receive_result =
      rx_socket_.ReceiveFrom(std::span<std::byte>(rx_datagram_buffer_.data(),
                                                  read_capacity));
  if (receive_result.status != tempolink::net::SocketStatus::Success ||
      receive_result.bytes_read == 0U) {
    return false;
  }

  tempolink::bridge::ParsedUdpBridgePacket packet;
  if (!tempolink::bridge::ParseUdpBridgePacket(
          std::span<const std::byte>(rx_datagram_buffer_.data(),
                                     receive_result.bytes_read),
          packet)) {
    return false;
  }
  if (packet.header.type != tempolink::bridge::UdpBridgePacketType::kDawToClient) {
    return false;
  }
  if (packet.header.sample_rate_hz != sample_rate_hz) {
    return false;
  }
  if (packet.samples.size() < expected_samples) {
    return false;
  }

  out.assign(packet.samples.begin(), packet.samples.begin() + expected_samples);
  return true;
}

bool UdpAudioBridgePort::SendToPlugin(
    tempolink::bridge::UdpBridgePacketType type,
    std::span<const std::int16_t> samples, std::uint32_t sample_rate_hz,
    std::uint16_t channels, std::uint16_t frames) {
  if (samples.empty()) {
    return false;
  }
  if (!EnsureSocketsReady()) {
    return false;
  }

  tempolink::bridge::UdpBridgePacketHeader header;
  header.type = type;
  header.sample_rate_hz = sample_rate_hz;
  header.channels = channels;
  header.frames = frames;
  header.sequence = tx_sequence_++;

  const auto packet = tempolink::bridge::SerializeUdpBridgePacket(header, samples);

  std::scoped_lock lock(io_mutex_);
  return tx_socket_.SendTo(
      std::span<const std::byte>(packet.data(), packet.size()), host_,
      plugin_listen_port_);
}

}  // namespace tempolink::juceapp::bridge
