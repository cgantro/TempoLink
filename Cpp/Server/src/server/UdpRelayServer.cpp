#include "tempolink/server/UdpRelayServer.h"

#include <chrono>
#include <iostream>
#include <thread>

#include "tempolink/net/PacketCodec.h"

namespace tempolink::server {
namespace {

using tempolink::net::Packet;
using tempolink::net::PacketType;

std::uint64_t NowMicros() {
  const auto now = std::chrono::steady_clock::now().time_since_epoch();
  return static_cast<std::uint64_t>(
      std::chrono::duration_cast<std::chrono::microseconds>(now).count());
}

}  // namespace

UdpRelayServer::UdpRelayServer(std::uint16_t port) : port_(port) {}

bool UdpRelayServer::Start() {
  if (running_) {
    return true;
  }

  if (!socket_.Open()) {
    std::cerr << "[server] failed to open socket: "
              << tempolink::net::UdpSocket::LastErrorMessage() << "\n";
    return false;
  }
  if (!socket_.Bind(port_)) {
    std::cerr << "[server] failed to bind UDP port " << port_ << ": "
              << tempolink::net::UdpSocket::LastErrorMessage() << "\n";
    return false;
  }
  if (!socket_.SetNonBlocking(true)) {
    std::cerr << "[server] failed to set non-blocking mode\n";
    return false;
  }

  running_ = true;
  std::cout << "[server] listening on UDP :" << port_ << "\n";
  return true;
}

void UdpRelayServer::Run() {
  using namespace std::chrono_literals;

  auto last_cleanup = std::chrono::steady_clock::now();
  while (running_) {
    auto datagram = socket_.ReceiveFrom();
    if (datagram.has_value()) {
      HandleDatagram(*datagram);
    } else {
      std::this_thread::sleep_for(1ms);
    }

    const auto now = std::chrono::steady_clock::now();
    if (now - last_cleanup >= 5s) {
      room_registry_.RemoveStale(std::chrono::seconds(15));
      last_cleanup = now;
    }
  }
}

void UdpRelayServer::Stop() { running_ = false; }

void UdpRelayServer::HandleDatagram(const tempolink::net::Datagram& datagram) {
  auto packet = tempolink::net::DecodePacket(datagram.data);
  if (!packet.has_value()) {
    return;
  }

  switch (packet->header.type) {
    case PacketType::kJoin:
      HandleJoin(*packet, datagram);
      break;
    case PacketType::kLeave:
      HandleLeave(*packet);
      break;
    case PacketType::kPing:
      HandlePing(*packet, datagram);
      break;
    case PacketType::kClockSync:
      HandleClockSync(*packet, datagram);
      break;
    case PacketType::kAudio:
      HandleAudio(*packet, datagram);
      break;
    case PacketType::kHeartbeat:
      room_registry_.Upsert(
          packet->header.room_id,
          ParticipantEndpoint{packet->header.sender_id, datagram.host,
                              datagram.port, std::chrono::steady_clock::now()});
      break;
    case PacketType::kPong:
    case PacketType::kClockSyncAck:
    case PacketType::kStats:
    case PacketType::kControl:
      break;
  }
}

void UdpRelayServer::HandleJoin(const Packet& packet,
                                const tempolink::net::Datagram& datagram) {
  room_registry_.Upsert(packet.header.room_id,
                        ParticipantEndpoint{
                            packet.header.sender_id,
                            datagram.host,
                            datagram.port,
                            std::chrono::steady_clock::now(),
                        });

  Packet ack;
  ack.header.type = PacketType::kControl;
  ack.header.room_id = packet.header.room_id;
  ack.header.sender_id = 0;
  ack.header.timestamp_us = NowMicros();
  const std::string payload_text = "JOIN_OK";
  ack.payload.resize(payload_text.size());
  for (std::size_t i = 0; i < payload_text.size(); ++i) {
    ack.payload[i] = static_cast<std::byte>(payload_text[i]);
  }

  const auto encoded = tempolink::net::EncodePacket(ack);
  socket_.SendTo(encoded, datagram.host, datagram.port);

  std::cout << "[server] join room=" << packet.header.room_id
            << " user=" << packet.header.sender_id << " from " << datagram.host
            << ":" << datagram.port << "\n";
}

void UdpRelayServer::HandleLeave(const Packet& packet) {
  room_registry_.Remove(packet.header.room_id, packet.header.sender_id);
  std::cout << "[server] leave room=" << packet.header.room_id
            << " user=" << packet.header.sender_id << "\n";
}

void UdpRelayServer::HandlePing(const Packet& packet,
                                const tempolink::net::Datagram& datagram) {
  Packet pong = packet;
  pong.header.type = PacketType::kPong;
  pong.header.timestamp_us = packet.header.timestamp_us;

  const auto encoded = tempolink::net::EncodePacket(pong);
  socket_.SendTo(encoded, datagram.host, datagram.port);
}

void UdpRelayServer::HandleClockSync(
    const Packet& packet, const tempolink::net::Datagram& datagram) {
  const auto server_receive_us = NowMicros();
  auto response =
      clock_sync_.BuildClockSyncAck(packet, server_receive_us, NowMicros());
  const auto encoded = tempolink::net::EncodePacket(response);
  socket_.SendTo(encoded, datagram.host, datagram.port);
}

void UdpRelayServer::HandleAudio(const Packet& packet,
                                 const tempolink::net::Datagram& datagram) {
  room_registry_.Upsert(packet.header.room_id,
                        ParticipantEndpoint{
                            packet.header.sender_id,
                            datagram.host,
                            datagram.port,
                            std::chrono::steady_clock::now(),
                        });

  const auto peers =
      room_registry_.GetPeers(packet.header.room_id, packet.header.sender_id);
  if (peers.empty()) {
    return;
  }

  const auto encoded = tempolink::net::EncodePacket(packet);
  for (const auto& peer : peers) {
    socket_.SendTo(encoded, peer.host, peer.port);
  }
}

}  // namespace tempolink::server
