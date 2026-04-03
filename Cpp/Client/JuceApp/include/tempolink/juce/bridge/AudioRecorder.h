#pragma once

#include <cstdint>
#include <mutex>
#include <span>
#include <vector>

#include <juce_core/juce_core.h>

#include "tempolink/audio/IAudioOutputDevice.h"

namespace tempolink::juceapp::bridge {

class AudioRecorder final {
 public:
  bool Start(juce::String& output_path, juce::String& error_text);
  bool Stop(juce::String& output_path, juce::String& error_text);
  bool IsRecording() const;

  void ConsumePlayback(std::span<const std::int16_t> frame,
                       const tempolink::audio::AudioPlaybackConfig& config);

 private:
  mutable std::mutex mutex_;
  bool recording_active_ = false;
  std::vector<std::int16_t> samples_;
  std::uint32_t sample_rate_hz_ = 0;
  std::uint16_t channels_ = 0;
  juce::File target_file_;
};

}  // namespace tempolink::juceapp::bridge

