#pragma once

#include <span>
#include <vector>

#include "tempolink/client/audio/AudioProcessor.h"

namespace tempolink::client::audio {

/// Runs a chain of AudioProcessor instances in sequence on a PCM buffer.
class AudioProcessorChain {
 public:
  /// Add a processor to the end of the chain. Does not take ownership.
  void AddProcessor(AudioProcessor* processor);

  /// Remove a processor from the chain. Returns true if found and removed.
  bool RemoveProcessor(AudioProcessor* processor);

  /// Process the buffer through all registered processors in order.
  void Process(std::span<float> pcm, const AudioFormat& format);

  /// Clear all processors.
  void Clear();

  /// Number of processors in the chain.
  std::size_t Size() const;

 private:
  std::vector<AudioProcessor*> processors_;
};

}  // namespace tempolink::client::audio
