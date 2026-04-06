#include "tempolink/audio/OpusCodec.h"

#ifdef TEMPOLINK_HAS_OPUS
#include <opus/opus.h>
#endif

#include <algorithm>
#include <cstddef>
#include <cstring>

namespace tempolink::audio {

OpusCodec::OpusCodec() = default;

OpusCodec::~OpusCodec() {
#ifdef TEMPOLINK_HAS_OPUS
  if (encoder_ != nullptr) {
    opus_encoder_destroy(encoder_);
    encoder_ = nullptr;
  }
  if (decoder_ != nullptr) {
    opus_decoder_destroy(decoder_);
    decoder_ = nullptr;
  }
#endif
}

bool OpusCodec::Initialize(std::uint32_t sample_rate_hz, std::uint8_t channels,
                           std::uint16_t frame_size) {
  sample_rate_hz_ = sample_rate_hz;
  channels_ = std::max<std::uint8_t>(1, channels);
  frame_size_ = frame_size;

#ifdef TEMPOLINK_HAS_OPUS
  int error = OPUS_OK;
  encoder_ =
      opus_encoder_create(static_cast<int>(sample_rate_hz_), channels_,
                          OPUS_APPLICATION_RESTRICTED_LOWDELAY, &error);
  if (error != OPUS_OK || encoder_ == nullptr) {
    encoder_ = nullptr;
    return false;
  }

  decoder_ = opus_decoder_create(static_cast<int>(sample_rate_hz_), channels_,
                                 &error);
  if (error != OPUS_OK || decoder_ == nullptr) {
    opus_encoder_destroy(encoder_);
    encoder_ = nullptr;
    decoder_ = nullptr;
    return false;
  }

  opus_encoder_ctl(encoder_, OPUS_SET_BITRATE(64000));
  opus_encoder_ctl(encoder_, OPUS_SET_SIGNAL(OPUS_SIGNAL_MUSIC));
  return true;
#else
  return false;
#endif
}

std::vector<std::byte> OpusCodec::Encode(std::span<const float> pcm) {
#ifdef TEMPOLINK_HAS_OPUS
  if (encoder_ == nullptr || pcm.empty()) {
    return {};
  }

  const std::size_t required_samples =
      static_cast<std::size_t>(frame_size_) * static_cast<std::size_t>(channels_);
  if (required_samples == 0 || pcm.size() < required_samples) {
    return {};
  }

  std::vector<unsigned char> encoded(4000);
  const int result =
      opus_encode_float(encoder_, pcm.data(), frame_size_, encoded.data(),
                        static_cast<opus_int32>(encoded.size()));
  if (result <= 0) {
    return {};
  }

  std::vector<std::byte> bytes(static_cast<std::size_t>(result));
  std::memcpy(bytes.data(), encoded.data(), static_cast<std::size_t>(result));
  return bytes;
#else
  (void)pcm;
  return {};
#endif
}

std::vector<float> OpusCodec::Decode(std::span<const std::byte> encoded) {
#ifdef TEMPOLINK_HAS_OPUS
  if (decoder_ == nullptr || encoded.empty()) {
    return {};
  }

  std::vector<float> pcm(
      static_cast<std::size_t>(frame_size_) * channels_);
  const int decoded = opus_decode_float(
      decoder_, reinterpret_cast<const unsigned char*>(encoded.data()),
      static_cast<opus_int32>(encoded.size()),
      pcm.data(), frame_size_, 0);
  if (decoded <= 0) {
    return {};
  }

  pcm.resize(static_cast<std::size_t>(decoded) * channels_);
  return pcm;
#else
  (void)encoded;
  return {};
#endif
}

bool OpusCodec::SetBitrate(int bitrate) {
#ifdef TEMPOLINK_HAS_OPUS
  if (encoder_ == nullptr) {
    return false;
  }
  return opus_encoder_ctl(encoder_, OPUS_SET_BITRATE(bitrate)) == OPUS_OK;
#else
  (void)bitrate;
  return false;
#endif
}

bool OpusCodec::SetComplexity(int complexity) {
#ifdef TEMPOLINK_HAS_OPUS
  if (encoder_ == nullptr) {
    return false;
  }
  return opus_encoder_ctl(encoder_, OPUS_SET_COMPLEXITY(complexity)) == OPUS_OK;
#else
  (void)complexity;
  return false;
#endif
}

}  // namespace tempolink::audio
