#include "tempolink/server/ClockSyncService.h"

namespace tempolink::server {
namespace {

void WriteU64(std::byte* out, std::uint64_t value) {
  out[0] = static_cast<std::byte>((value >> 56) & 0xFF);
  out[1] = static_cast<std::byte>((value >> 48) & 0xFF);
  out[2] = static_cast<std::byte>((value >> 40) & 0xFF);
  out[3] = static_cast<std::byte>((value >> 32) & 0xFF);
  out[4] = static_cast<std::byte>((value >> 24) & 0xFF);
  out[5] = static_cast<std::byte>((value >> 16) & 0xFF);
  out[6] = static_cast<std::byte>((value >> 8) & 0xFF);
  out[7] = static_cast<std::byte>(value & 0xFF);
}

std::uint64_t ReadU64(const std::byte* in) {
  return (static_cast<std::uint64_t>(std::to_integer<unsigned int>(in[0])) << 56) |
         (static_cast<std::uint64_t>(std::to_integer<unsigned int>(in[1])) << 48) |
         (static_cast<std::uint64_t>(std::to_integer<unsigned int>(in[2])) << 40) |
         (static_cast<std::uint64_t>(std::to_integer<unsigned int>(in[3])) << 32) |
         (static_cast<std::uint64_t>(std::to_integer<unsigned int>(in[4])) << 24) |
         (static_cast<std::uint64_t>(std::to_integer<unsigned int>(in[5])) << 16) |
         (static_cast<std::uint64_t>(std::to_integer<unsigned int>(in[6])) << 8) |
         static_cast<std::uint64_t>(std::to_integer<unsigned int>(in[7]));
}

}  // namespace

tempolink::net::Packet ClockSyncService::BuildClockSyncAck(
    const tempolink::net::Packet& request, std::uint64_t server_receive_us,
    std::uint64_t server_send_us) const {
  tempolink::net::Packet response;
  response.header.type = tempolink::net::PacketType::kClockSyncAck;
  response.header.room_id = request.header.room_id;
  response.header.sender_id = 0;
  response.header.sequence = request.header.sequence;
  response.header.timestamp_us = server_send_us;

  response.payload.resize(kClockSyncAckSize);
  const auto client_send_us = ReadClientSendTimestamp(request);
  WriteU64(response.payload.data() + 0, client_send_us);
  WriteU64(response.payload.data() + 8, server_receive_us);
  WriteU64(response.payload.data() + 16, server_send_us);
  return response;
}

std::uint64_t ClockSyncService::ReadClientSendTimestamp(
    const tempolink::net::Packet& request) {
  if (request.payload.size() >= kClockSyncRequestSize) {
    return ReadU64(request.payload.data());
  }
  return request.header.timestamp_us;
}

}  // namespace tempolink::server

