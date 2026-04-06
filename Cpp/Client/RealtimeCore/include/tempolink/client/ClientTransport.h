#pragma once

#include <cstddef>
#include <cstdint>
#include <atomic>
#include <mutex>
#include <span>
#include <string>

#include <vector>
#include <array>

#include "tempolink/net/Packet.h"
#include "tempolink/net/UdpSocket.h"

namespace tempolink::client {

class ClientTransport {
 public:
  struct Endpoint {
    std::string server_host;
    std::uint16_t server_port = 0;
    std::uint32_t room_id = 0;
    std::uint32_t participant_id = 0;
  };

  bool Start(const Endpoint& endpoint);
  void Stop();
  bool IsRunning() const;

  bool SendPacket(tempolink::net::PacketType type, std::span<const std::byte> payload);
  bool SendTextPacket(tempolink::net::PacketType type, const std::string& text_payload);
  bool PollPacket(tempolink::net::Packet& packet);

 private:
  Endpoint endpoint_{};
  std::atomic<std::uint32_t> sequence_{0};
  std::atomic_bool running_{false};
  tempolink::net::UdpSocket socket_;
  std::mutex send_mutex_;
  std::array<std::byte, 16384> send_buffer_;
  std::array<std::byte, 16384> recv_buffer_;
};

}  // namespace tempolink::client
