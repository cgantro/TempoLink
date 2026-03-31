#include "tempolink/audio/AudioCodecFactory.h"

#include <memory>

#include "tempolink/audio/NullAudioCodec.h"
#include "tempolink/audio/OpusCodec.h"

namespace tempolink::audio {

std::unique_ptr<IAudioCodec> CreateDefaultAudioCodec() {
#ifdef TEMPOLINK_HAS_OPUS
  return std::make_unique<OpusCodec>();
#else
  return std::make_unique<NullAudioCodec>();
#endif
}

}  // namespace tempolink::audio
