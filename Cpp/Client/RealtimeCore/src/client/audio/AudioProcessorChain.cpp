#include "tempolink/client/audio/AudioProcessorChain.h"

#include <algorithm>

namespace tempolink::client::audio {

void AudioProcessorChain::AddProcessor(AudioProcessor* processor) {
  if (processor != nullptr) {
    processors_.push_back(processor);
  }
}

bool AudioProcessorChain::RemoveProcessor(AudioProcessor* processor) {
  auto it = std::find(processors_.begin(), processors_.end(), processor);
  if (it == processors_.end()) {
    return false;
  }
  processors_.erase(it);
  return true;
}

void AudioProcessorChain::Process(std::span<float> pcm, const AudioFormat& format) {
  for (auto* processor : processors_) {
    processor->Process(pcm, format);
  }
}

void AudioProcessorChain::Clear() {
  processors_.clear();
}

std::size_t AudioProcessorChain::Size() const {
  return processors_.size();
}

}  // namespace tempolink::client::audio
