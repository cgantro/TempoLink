#pragma once

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <span>
#include <string>

#include "tempolink/net/Packet.h"
#include "tempolink/net/UdpSocket.h"

namespace tempolink::client {

class ClientTransport {
 public:
  struct Endpoint {
    std::string server_host = "127.0.0.1";
    std::uint16_t server_port = 40000;
    std::uint32_t room_id = 1;
    std::uint32_t participant_id = 1001;
  };

  bool Start(const Endpoint& endpoint);
  void Stop();
  bool IsRunning() const;

  bool SendPacket(tempolink::net::PacketType type, std::span<const std::byte> payload);
  bool SendTextPacket(tempolink::net::PacketType type, const std::string& text_payload);
  bool PollPacket(tempolink::net::Packet& packet);

 private:
  Endpoint endpoint_{};
  std::uint32_t sequence_ = 0;
  bool running_ = false;
  tempolink::net::UdpSocket socket_;
  std::mutex send_mutex_;
};

}  // namespace tempolink::client
