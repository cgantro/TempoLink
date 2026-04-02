#pragma once

#include <cstdint>
#include <mutex>
#include <span>
#include <unordered_map>
#include <vector>

#include "tempolink/client/audio/AudioProcessor.h"

namespace tempolink::client::audio {

class MultiStreamMixer final {
 public:
  struct PeerSettings {
    float volume = 1.0F;
    float pan = 0.0F; // -1.0 (left) to 1.0 (right)
  };

  /**
   * @brief Mix a peer's audio into a master buffer.
   * @param master_pcm The output buffer to mix into (must be pre-allocated).
   * @param peer_pcm The input buffer from the peer.
   * @param participant_id The ID of the peer.
   * @param format The audio format.
   */
  void MixPeer(std::span<float> master_pcm, std::span<const float> peer_pcm,
               std::uint32_t participant_id, const AudioFormat& format);

  void SetPeerVolume(std::uint32_t participant_id, float volume);
  float PeerVolume(std::uint32_t participant_id) const;
  void SetPeerPan(std::uint32_t participant_id, float pan);
  float PeerPan(std::uint32_t participant_id) const;

  void RemovePeer(std::uint32_t participant_id);

 private:
  void ComputePanGains(float pan, float& left, float& right) const;

  mutable std::mutex mutex_;
  std::unordered_map<std::uint32_t, PeerSettings> peer_settings_;
};

}  // namespace tempolink::client::audio
