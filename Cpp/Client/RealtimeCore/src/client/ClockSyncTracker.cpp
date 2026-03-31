#include "tempolink/client/ClockSyncTracker.h"

#include <algorithm>

namespace tempolink::client {
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

std::vector<std::byte> ClockSyncTracker::BuildRequestPayload(
    std::uint64_t client_send_us) const {
  std::vector<std::byte> payload(8);
  WriteU64(payload.data(), client_send_us);
  return payload;
}

bool ClockSyncTracker::HandleAck(std::span<const std::byte> ack_payload,
                                 std::uint64_t client_receive_us) {
  if (ack_payload.size() < 24) {
    return false;
  }

  const std::uint64_t t0 = ReadU64(ack_payload.data() + 0);   // client send
  const std::uint64_t t1 = ReadU64(ack_payload.data() + 8);   // server recv
  const std::uint64_t t2 = ReadU64(ack_payload.data() + 16);  // server send
  const std::uint64_t t3 = client_receive_us;                 // client recv

  if (t3 < t0 || t2 < t1) {
    return false;
  }

  const auto total_path_us =
      static_cast<std::int64_t>(t3) - static_cast<std::int64_t>(t0);
  const auto server_processing_us =
      static_cast<std::int64_t>(t2) - static_cast<std::int64_t>(t1);
  const auto rtt_us = std::max<std::int64_t>(0, total_path_us - server_processing_us);
  const auto raw_rtt_ms = static_cast<std::uint32_t>(rtt_us / 1000LL);

  const auto part_a = static_cast<std::int64_t>(t1) - static_cast<std::int64_t>(t0);
  const auto part_b = static_cast<std::int64_t>(t2) - static_cast<std::int64_t>(t3);
  const auto raw_offset_us = (part_a + part_b) / 2;

  if (!has_estimate_) {
    rtt_ms_ = raw_rtt_ms;
    clock_offset_us_ = raw_offset_us;
    has_estimate_ = true;
    return true;
  }

  // EWMA smoothing to avoid abrupt playout shifts.
  rtt_ms_ = static_cast<std::uint32_t>((rtt_ms_ * 7U + raw_rtt_ms * 3U) / 10U);
  clock_offset_us_ = (clock_offset_us_ * 7 + raw_offset_us * 3) / 10;
  return true;
}

bool ClockSyncTracker::HasEstimate() const { return has_estimate_; }

std::int64_t ClockSyncTracker::ClockOffsetUs() const { return clock_offset_us_; }

std::uint32_t ClockSyncTracker::RttMs() const { return rtt_ms_; }

}  // namespace tempolink::client
