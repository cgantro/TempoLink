#pragma once

#include <cstdint>

#include "tempolink/net/Packet.h"

namespace tempolink::server {

class ClockSyncService {
 public:
  static constexpr std::size_t kClockSyncRequestSize = 8;
  static constexpr std::size_t kClockSyncAckSize = 24;

  tempolink::net::Packet BuildClockSyncAck(
      const tempolink::net::Packet& request, std::uint64_t server_receive_us,
      std::uint64_t server_send_us) const;

 private:
  static std::uint64_t ReadClientSendTimestamp(
      const tempolink::net::Packet& request);
};

}  // namespace tempolink::server

