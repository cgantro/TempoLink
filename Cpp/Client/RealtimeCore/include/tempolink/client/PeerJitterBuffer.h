#pragma once

#include <cstddef>
#include <cstdint>
#include <array>
#include <span>
#include <vector>

namespace tempolink::client {

class PeerJitterBuffer {
 public:
  struct EncodedFrame {
    std::uint32_t sequence = 0;
    std::uint64_t capture_timestamp_us = 0;
    std::vector<std::byte> payload;
  };

  void Push(std::uint32_t sequence, std::uint64_t capture_timestamp_us,
            std::span<const std::byte> payload);
  void Push(std::uint32_t sequence, std::uint64_t capture_timestamp_us,
            std::vector<std::byte>&& payload);
  std::vector<EncodedFrame> PopReady(std::size_t target_depth_packets);
  void Reset();

 private:
  static constexpr std::size_t kSlotCount = 64;

  struct Slot {
    bool occupied = false;
    EncodedFrame frame;
  };

  std::array<Slot, kSlotCount> slots_{};
  std::size_t stored_count_ = 0;
  std::uint32_t expected_sequence_ = 0;
  bool expected_sequence_initialized_ = false;
  bool primed_ = false;
};

}  // namespace tempolink::client
