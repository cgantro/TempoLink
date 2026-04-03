#include "tempolink/juce/bridge/AudioFilePlayer.h"

#include <algorithm>
#include <cmath>

#include <juce_audio_formats/juce_audio_formats.h>

namespace tempolink::juceapp::bridge {
namespace {

float Pcm16ToFloat(const std::int16_t sample) {
  return static_cast<float>(sample) / 32767.0F;
}

std::int16_t FloatToPcm16(const float sample) {
  const float clamped = std::clamp(sample, -1.0F, 1.0F);
  return static_cast<std::int16_t>(clamped * 32767.0F);
}

}  // namespace

bool AudioFilePlayer::LoadFile(const juce::File& file, juce::String& error_text) {
  juce::AudioFormatManager format_manager;
  format_manager.registerBasicFormats();
  std::unique_ptr<juce::AudioFormatReader> reader(format_manager.createReaderFor(file));
  if (!reader) {
    error_text = "Unsupported audio file format.";
    return false;
  }

  const auto length_in_samples = static_cast<int>(reader->lengthInSamples);
  const auto channels = static_cast<int>(std::max<unsigned int>(1U, reader->numChannels));
  if (length_in_samples <= 0 || channels <= 0) {
    error_text = "Audio file has no samples.";
    return false;
  }

  juce::AudioBuffer<float> buffer(channels, length_in_samples);
  if (!reader->read(&buffer, 0, length_in_samples, 0, true, true)) {
    error_text = "Failed to read audio samples.";
    return false;
  }

  std::vector<float> interleaved;
  interleaved.resize(static_cast<std::size_t>(length_in_samples) *
                     static_cast<std::size_t>(channels));
  for (int sample = 0; sample < length_in_samples; ++sample) {
    for (int ch = 0; ch < channels; ++ch) {
      interleaved[static_cast<std::size_t>(sample) * static_cast<std::size_t>(channels) +
                  static_cast<std::size_t>(ch)] = buffer.getSample(ch, sample);
    }
  }

  std::lock_guard<std::mutex> lock(mutex_);
  samples_ = std::move(interleaved);
  channels_ = static_cast<std::size_t>(channels);
  sample_rate_ = reader->sampleRate;
  position_frames_ = 0.0;
  active_ = true;
  label_ = file.getFileName();
  return true;
}

void AudioFilePlayer::Stop() {
  std::lock_guard<std::mutex> lock(mutex_);
  active_ = false;
  position_frames_ = 0.0;
}

bool AudioFilePlayer::IsActive() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return active_;
}

bool AudioFilePlayer::SetPosition(float normalized_position) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (samples_.empty() || channels_ == 0) {
    return false;
  }
  const std::size_t total_frames = samples_.size() / channels_;
  if (total_frames == 0) {
    return false;
  }
  const double clamped = std::clamp(static_cast<double>(normalized_position), 0.0, 1.0);
  position_frames_ = clamped * static_cast<double>(total_frames - 1U);
  return true;
}

float AudioFilePlayer::Position() const {
  std::lock_guard<std::mutex> lock(mutex_);
  if (samples_.empty() || channels_ == 0) {
    return 0.0F;
  }
  const std::size_t total_frames = samples_.size() / channels_;
  if (total_frames == 0) {
    return 0.0F;
  }
  const double pos = position_frames_ / static_cast<double>(total_frames);
  return static_cast<float>(std::clamp(pos, 0.0, 1.0));
}

void AudioFilePlayer::SetLoopEnabled(bool enabled) {
  std::lock_guard<std::mutex> lock(mutex_);
  loop_enabled_ = enabled;
}

bool AudioFilePlayer::IsLoopEnabled() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return loop_enabled_;
}

juce::String AudioFilePlayer::CurrentLabel() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return label_;
}

void AudioFilePlayer::MixCapturedInput(
    std::span<std::int16_t> frame,
    const tempolink::audio::AudioCaptureConfig& config) {
  std::unique_lock<std::mutex> lock(mutex_, std::try_to_lock);
  if (!lock.owns_lock() || !active_ || samples_.empty() || channels_ == 0 ||
      config.sample_rate_hz == 0 || config.channels == 0) {
    return;
  }

  const std::size_t dst_channels = std::max<std::size_t>(1, config.channels);
  const std::size_t dst_frames = frame.size() / dst_channels;
  const std::size_t src_frames = samples_.size() / channels_;
  if (dst_frames == 0 || src_frames == 0) {
    return;
  }

  const double frame_step = sample_rate_ / static_cast<double>(config.sample_rate_hz);
  if (frame_step <= 0.0) {
    return;
  }

  for (std::size_t frame_index = 0; frame_index < dst_frames; ++frame_index) {
    std::size_t src_frame_index = static_cast<std::size_t>(position_frames_);
    if (src_frame_index >= src_frames) {
      if (loop_enabled_) {
        position_frames_ =
            std::fmod(position_frames_, static_cast<double>(std::max<std::size_t>(1, src_frames)));
        src_frame_index = static_cast<std::size_t>(position_frames_);
      } else {
        active_ = false;
        break;
      }
    }

    const std::size_t next_src_frame_index = std::min(src_frames - 1U, src_frame_index + 1U);
    const double frac = std::clamp(position_frames_ - static_cast<double>(src_frame_index), 0.0, 1.0);

    for (std::size_t ch = 0; ch < dst_channels; ++ch) {
      const std::size_t src_ch = std::min<std::size_t>(ch, channels_ - 1U);
      const float s0 = samples_[src_frame_index * channels_ + src_ch];
      const float s1 = samples_[next_src_frame_index * channels_ + src_ch];
      const float source_sample = s0 + (s1 - s0) * static_cast<float>(frac);
      const std::size_t dst_index = frame_index * dst_channels + ch;
      const float mixed = Pcm16ToFloat(frame[dst_index]) + source_sample;
      frame[dst_index] = FloatToPcm16(mixed);
    }

    position_frames_ += frame_step;
  }
}

}  // namespace tempolink::juceapp::bridge

