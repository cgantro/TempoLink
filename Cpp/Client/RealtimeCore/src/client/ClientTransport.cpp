#include "tempolink/client/ClientTransport.h"

#include <chrono>
#include <utility>
#include <vector>

#include "tempolink/net/PacketCodec.h"

namespace tempolink::client {
namespace {

std::uint64_t NowMicros() {
  const auto now = std::chrono::steady_clock::now().time_since_epoch();
  return static_cast<std::uint64_t>(
      std::chrono::duration_cast<std::chrono::microseconds>(now).count());
}

}  // namespace

bool ClientTransport::Start(const Endpoint& endpoint) {
  Stop();
  endpoint_ = endpoint;
  sequence_ = 0;

  if (!socket_.Open()) {
    return false;
  }
  if (!socket_.Bind(0)) {
    socket_.Close();
    return false;
  }
  if (!socket_.SetNonBlocking(true)) {
    socket_.Close();
    return false;
  }

  running_ = true;
  return true;
}

void ClientTransport::Stop() {
  if (!running_) {
    return;
  }
  running_ = false;
  socket_.Close();
}

bool ClientTransport::IsRunning() const { return running_; }

bool ClientTransport::SendPacket(tempolink::net::PacketType type,
                                 std::span<const std::byte> payload) {
  if (!running_) {
    return false;
  }

  tempolink::net::Packet packet;
  packet.header.type = type;
  packet.header.room_id = endpoint_.room_id;
  packet.header.sender_id = endpoint_.participant_id;
  packet.header.sequence = ++sequence_;
  packet.header.timestamp_us = NowMicros();
  packet.payload.assign(payload.begin(), payload.end());

  const auto encoded = tempolink::net::EncodePacket(packet);
  std::scoped_lock lock(send_mutex_);
  return socket_.SendTo(encoded, endpoint_.server_host, endpoint_.server_port);
}

bool ClientTransport::SendTextPacket(tempolink::net::PacketType type,
                                     const std::string& text_payload) {
  std::vector<std::byte> bytes(text_payload.size());
  for (std::size_t i = 0; i < text_payload.size(); ++i) {
    bytes[i] = static_cast<std::byte>(text_payload[i]);
  }
  return SendPacket(type, bytes);
}

bool ClientTransport::PollPacket(tempolink::net::Packet& packet) {
  if (!running_) {
    return false;
  }

  while (true) {
    auto datagram = socket_.ReceiveFrom();
    if (!datagram.has_value()) {
      return false;
    }

    auto decoded = tempolink::net::DecodePacket(datagram->data);
    if (!decoded.has_value()) {
      continue;
    }

    packet = std::move(decoded.value());
    return true;
  }
}

}  // namespace tempolink::client
