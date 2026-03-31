#pragma once

#include <atomic>
#include <cstdint>

#include "tempolink/net/Packet.h"
#include "tempolink/net/UdpSocket.h"
#include "tempolink/server/ClockSyncService.h"
#include "tempolink/server/RoomRegistry.h"

namespace tempolink::server {

class UdpRelayServer {
 public:
  explicit UdpRelayServer(std::uint16_t port);

  bool Start();
  void Run();
  void Stop();

 private:
  void HandleDatagram(const tempolink::net::Datagram& datagram);
  void HandleJoin(const tempolink::net::Packet& packet,
                  const tempolink::net::Datagram& datagram);
  void HandleLeave(const tempolink::net::Packet& packet);
  void HandlePing(const tempolink::net::Packet& packet,
                  const tempolink::net::Datagram& datagram);
  void HandleClockSync(const tempolink::net::Packet& packet,
                       const tempolink::net::Datagram& datagram);
  void HandleAudio(const tempolink::net::Packet& packet,
                   const tempolink::net::Datagram& datagram);

  std::uint16_t port_ = 0;
  std::atomic_bool running_{false};
  tempolink::net::UdpSocket socket_;
  RoomRegistry room_registry_;
  ClockSyncService clock_sync_;
};

}  // namespace tempolink::server
