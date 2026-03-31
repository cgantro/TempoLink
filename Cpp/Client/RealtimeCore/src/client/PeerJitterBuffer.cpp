#include "tempolink/client/PeerJitterBuffer.h"

#include <utility>

namespace tempolink::client {

void PeerJitterBuffer::Push(std::uint32_t sequence,
                            std::uint64_t capture_timestamp_us,
                            std::span<const std::byte> payload) {
  if (frames_.contains(sequence)) {
    return;
  }

  EncodedFrame frame;
  frame.sequence = sequence;
  frame.capture_timestamp_us = capture_timestamp_us;
  frame.payload.assign(payload.begin(), payload.end());
  frames_.emplace(sequence, std::move(frame));

  if (!expected_sequence_initialized_) {
    expected_sequence_ = sequence;
    expected_sequence_initialized_ = true;
  }
}

std::vector<PeerJitterBuffer::EncodedFrame> PeerJitterBuffer::PopReady(
    std::size_t target_depth_packets) {
  std::vector<EncodedFrame> ready;
  if (!expected_sequence_initialized_ || frames_.empty()) {
    return ready;
  }

  if (!primed_) {
    if (frames_.size() < target_depth_packets) {
      return ready;
    }
    primed_ = true;
  }

  while (!frames_.empty()) {
    auto it = frames_.find(expected_sequence_);
    if (it != frames_.end()) {
      ready.push_back(std::move(it->second));
      frames_.erase(it);
      ++expected_sequence_;
      continue;
    }

    // If a frame is missing for too long, advance sequence to avoid stalling.
    if (frames_.size() > target_depth_packets * 2U) {
      ++expected_sequence_;
      continue;
    }
    break;
  }

  return ready;
}

void PeerJitterBuffer::Reset() {
  frames_.clear();
  expected_sequence_ = 0;
  expected_sequence_initialized_ = false;
  primed_ = false;
}

}  // namespace tempolink::client

