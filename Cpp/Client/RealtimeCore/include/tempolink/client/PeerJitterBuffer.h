#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
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
  std::vector<EncodedFrame> PopReady(std::size_t target_depth_packets);
  void Reset();

 private:
  std::map<std::uint32_t, EncodedFrame> frames_;
  std::uint32_t expected_sequence_ = 0;
  bool expected_sequence_initialized_ = false;
  bool primed_ = false;
};

}  // namespace tempolink::client

