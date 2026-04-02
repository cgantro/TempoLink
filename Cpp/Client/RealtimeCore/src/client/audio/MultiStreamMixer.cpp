#include "tempolink/client/audio/MultiStreamMixer.h"

#include <algorithm>
#include <cmath>

namespace tempolink::client::audio {

void MultiStreamMixer::MixPeer(std::span<float> master_pcm,
                               std::span<const float> peer_pcm,
                               std::uint32_t participant_id,
                               const AudioFormat& format) {
  if (master_pcm.size() != peer_pcm.size()) {
    return;
  }

  PeerSettings settings;
  {
    std::scoped_lock lock(mutex_);
    auto it = peer_settings_.find(participant_id);
    if (it != peer_settings_.end()) {
      settings = it->second;
    } else {
      peer_settings_[participant_id] = settings;
    }
  }

  const float volume = std::clamp(settings.volume, 0.0F, 1.0F);
  const std::size_t channels = std::max<std::size_t>(1, format.channels);

  if (channels >= 2) {
    float left = 1.0F;
    float right = 1.0F;
    ComputePanGains(settings.pan, left, right);

    for (std::size_t i = 0; i < master_pcm.size(); i += channels) {
      master_pcm[i] += peer_pcm[i] * volume * left;
      if (i + 1 < master_pcm.size()) {
        master_pcm[i + 1] += peer_pcm[i + 1] * volume * right;
      }
      for (std::size_t c = 2; c < channels && i + c < master_pcm.size(); ++c) {
        master_pcm[i + c] += peer_pcm[i + c] * volume;
      }
    }
  } else {
    for (std::size_t i = 0; i < master_pcm.size(); ++i) {
      master_pcm[i] += peer_pcm[i] * volume;
    }
  }
}

void MultiStreamMixer::SetPeerVolume(std::uint32_t participant_id, float volume) {
  std::scoped_lock lock(mutex_);
  peer_settings_[participant_id].volume = std::clamp(volume, 0.0F, 1.0F);
}

float MultiStreamMixer::PeerVolume(std::uint32_t participant_id) const {
  std::scoped_lock lock(mutex_);
  auto it = peer_settings_.find(participant_id);
  if (it != peer_settings_.end()) {
    return it->second.volume;
  }
  return 1.0F;
}

void MultiStreamMixer::SetPeerPan(std::uint32_t participant_id, float pan) {
  std::scoped_lock lock(mutex_);
  peer_settings_[participant_id].pan = std::clamp(pan, -1.0F, 1.0F);
}

float MultiStreamMixer::PeerPan(std::uint32_t participant_id) const {
  std::scoped_lock lock(mutex_);
  auto it = peer_settings_.find(participant_id);
  if (it != peer_settings_.end()) {
    return it->second.pan;
  }
  return 0.0F;
}

void MultiStreamMixer::RemovePeer(std::uint32_t participant_id) {
  std::scoped_lock lock(mutex_);
  peer_settings_.erase(participant_id);
}

void MultiStreamMixer::ComputePanGains(float pan, float& left,
                                       float& right) const {
  const float p = std::clamp(pan, -1.0F, 1.0F);
  if (p < 0.0F) {
    left = 1.0F;
    right = 1.0F + p;
  } else if (p > 0.0F) {
    left = 1.0F - p;
    right = 1.0F;
  } else {
    left = 1.0F;
    right = 1.0F;
  }
}

}  // namespace tempolink::client::audio
