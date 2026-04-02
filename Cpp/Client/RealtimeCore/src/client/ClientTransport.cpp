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
  sequence_.store(0, std::memory_order_relaxed);

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

  running_.store(true, std::memory_order_release);
  return true;
}

void ClientTransport::Stop() {
  if (!running_.exchange(false, std::memory_order_acq_rel)) {
    return;
  }
  socket_.Close();
}

bool ClientTransport::IsRunning() const {
  return running_.load(std::memory_order_acquire);
}

bool ClientTransport::SendPacket(tempolink::net::PacketType type,
                                 std::span<const std::byte> payload) {
  if (!running_.load(std::memory_order_acquire)) {
    return false;
  }

  // Minimize heap allocations by using the pre-allocated send_buffer_
  // and encoding directly into it.
  tempolink::net::Packet packet;
  packet.header.type = type;
  packet.header.room_id = endpoint_.room_id;
  packet.header.sender_id = endpoint_.participant_id;
  packet.header.sequence = sequence_.fetch_add(1, std::memory_order_relaxed) + 1;
  packet.header.timestamp_us = NowMicros();
  
  // We avoid the vector copy by temporarily assigning the span
  // if Packet is refactored to allow it. For now, since EncodePacket
  // takes Packet, we might still have a copy if we keep using the struct.
  // Actually, let's optimize the encoding logic directly here or use a lighter struct.
  packet.payload.assign(payload.begin(), payload.end());

  std::scoped_lock lock(send_mutex_);
  const std::size_t encoded_size = tempolink::net::EncodePacket(packet, send_buffer_);
  if (encoded_size == 0) {
    return false;
  }

  return socket_.SendTo(std::span<const std::byte>(send_buffer_.data(), encoded_size),
                        endpoint_.server_host, endpoint_.server_port);
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
  if (!running_.load(std::memory_order_acquire)) {
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
