#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace tempolink::client {

class ClockSyncTracker {
 public:
  std::vector<std::byte> BuildRequestPayload(std::uint64_t client_send_us) const;
  bool HandleAck(std::span<const std::byte> ack_payload,
                 std::uint64_t client_receive_us);

  bool HasEstimate() const;
  std::int64_t ClockOffsetUs() const;
  std::uint32_t RttMs() const;

 private:
  std::int64_t clock_offset_us_ = 0;
  std::uint32_t rtt_ms_ = 0;
  bool has_estimate_ = false;
};

}  // namespace tempolink::client

