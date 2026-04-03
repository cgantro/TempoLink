#include "tempolink/client/PeerJitterBuffer.h"

#include <utility>

namespace tempolink::client {
void PeerJitterBuffer::Push(std::uint32_t sequence,
                            std::uint64_t capture_timestamp_us,
                            std::span<const std::byte> payload) {
  std::vector<std::byte> payload_copy(payload.begin(), payload.end());
  Push(sequence, capture_timestamp_us, std::move(payload_copy));
}

void PeerJitterBuffer::Push(std::uint32_t sequence,
                            std::uint64_t capture_timestamp_us,
                            std::vector<std::byte>&& payload) {
  if (!expected_sequence_initialized_) {
    expected_sequence_ = sequence;
    expected_sequence_initialized_ = true;
  }

  const std::size_t index = static_cast<std::size_t>(sequence % slots_.size());
  auto& slot = slots_[index];
  if (slot.occupied && slot.frame.sequence == sequence) {
    return;
  }
  if (slot.occupied) {
    slot.occupied = false;
    if (stored_count_ > 0) {
      --stored_count_;
    }
  }

  slot.frame.sequence = sequence;
  slot.frame.capture_timestamp_us = capture_timestamp_us;
  slot.frame.payload = std::move(payload);
  slot.occupied = true;
  ++stored_count_;
}

std::vector<PeerJitterBuffer::EncodedFrame> PeerJitterBuffer::PopReady(
    std::size_t target_depth_packets) {
  std::vector<EncodedFrame> ready;
  if (!expected_sequence_initialized_ || stored_count_ == 0) {
    return ready;
  }

  if (!primed_) {
    if (stored_count_ < target_depth_packets) {
      return ready;
    }
    primed_ = true;
  }

  while (stored_count_ > 0) {
    const std::size_t index = static_cast<std::size_t>(expected_sequence_ % slots_.size());
    auto& slot = slots_[index];
    if (slot.occupied && slot.frame.sequence == expected_sequence_) {
      ready.push_back(std::move(slot.frame));
      slot.occupied = false;
      if (stored_count_ > 0) {
        --stored_count_;
      }
      ++expected_sequence_;
      continue;
    }

    // If a frame is missing for too long, advance sequence to avoid stalling.
    if (stored_count_ > target_depth_packets * 2U) {
      ++expected_sequence_;
      continue;
    }
    break;
  }

  return ready;
}

void PeerJitterBuffer::Reset() {
  for (auto& slot : slots_) {
    slot.occupied = false;
    slot.frame.payload.clear();
  }
  stored_count_ = 0;
  expected_sequence_ = 0;
  expected_sequence_initialized_ = false;
  primed_ = false;
}

}  // namespace tempolink::client
