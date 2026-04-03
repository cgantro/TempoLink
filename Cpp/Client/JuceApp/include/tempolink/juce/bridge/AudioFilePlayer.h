#pragma once

#include <cstdint>
#include <mutex>
#include <span>
#include <vector>

#include <juce_core/juce_core.h>

#include "tempolink/audio/IAudioInputDevice.h"

namespace tempolink::juceapp::bridge {

class AudioFilePlayer final {
 public:
  bool LoadFile(const juce::File& file, juce::String& error_text);
  void Stop();
  bool IsActive() const;

  bool SetPosition(float normalized_position);
  float Position() const;
  void SetLoopEnabled(bool enabled);
  bool IsLoopEnabled() const;
  juce::String CurrentLabel() const;

  void MixCapturedInput(std::span<std::int16_t> frame,
                        const tempolink::audio::AudioCaptureConfig& config);

 private:
  mutable std::mutex mutex_;
  std::vector<float> samples_;
  std::size_t channels_ = 0;
  double sample_rate_ = 0.0;
  double position_frames_ = 0.0;
  bool active_ = false;
  bool loop_enabled_ = false;
  juce::String label_;
};

}  // namespace tempolink::juceapp::bridge

