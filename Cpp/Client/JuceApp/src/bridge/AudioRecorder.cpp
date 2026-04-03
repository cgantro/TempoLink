#include "tempolink/juce/bridge/AudioRecorder.h"

#include <juce_audio_formats/juce_audio_formats.h>

namespace tempolink::juceapp::bridge {
namespace {

float Pcm16ToFloat(const std::int16_t sample) {
  return static_cast<float>(sample) / 32767.0F;
}

}  // namespace

bool AudioRecorder::Start(juce::String& output_path, juce::String& error_text) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (recording_active_) {
    error_text = "Recording is already active.";
    return false;
  }

  const juce::File dir =
      juce::File::getSpecialLocation(juce::File::userMusicDirectory).getChildFile("SYNCROOM_REC");
  if (!dir.exists() && !dir.createDirectory()) {
    error_text = "Failed to create recording directory.";
    return false;
  }

  const juce::String stamp = juce::Time::getCurrentTime().formatted("%Y%m%d_%H%M%S");
  target_file_ = dir.getChildFile("syncroom_rec_" + stamp + ".wav");
  samples_.clear();
  samples_.reserve(48000 * 2 * 15);
  sample_rate_hz_ = 0;
  channels_ = 0;
  recording_active_ = true;
  output_path = target_file_.getFullPathName();
  return true;
}

bool AudioRecorder::Stop(juce::String& output_path, juce::String& error_text) {
  std::vector<std::int16_t> samples;
  std::uint32_t sample_rate_hz = 0;
  std::uint16_t channels = 0;
  juce::File target;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!recording_active_) {
      error_text = "Recording is not active.";
      return false;
    }
    recording_active_ = false;
    samples = std::move(samples_);
    sample_rate_hz = sample_rate_hz_;
    channels = channels_;
    target = target_file_;
    samples_.clear();
    sample_rate_hz_ = 0;
    channels_ = 0;
  }

  if (samples.empty() || sample_rate_hz == 0 || channels == 0) {
    error_text = "No audio captured for recording.";
    return false;
  }

  const int num_channels = static_cast<int>(channels);
  const int samples_per_channel =
      static_cast<int>(samples.size() / static_cast<std::size_t>(num_channels));
  if (samples_per_channel <= 0) {
    error_text = "Invalid recording buffer.";
    return false;
  }

  juce::AudioBuffer<float> audio_buffer(num_channels, samples_per_channel);
  for (int ch = 0; ch < num_channels; ++ch) {
    float* const dst = audio_buffer.getWritePointer(ch);
    for (int i = 0; i < samples_per_channel; ++i) {
      const std::size_t index =
          static_cast<std::size_t>(i) * static_cast<std::size_t>(num_channels) +
          static_cast<std::size_t>(ch);
      dst[i] = Pcm16ToFloat(samples[index]);
    }
  }

  std::unique_ptr<juce::FileOutputStream> stream(target.createOutputStream());
  if (!stream) {
    error_text = "Failed to open recording file.";
    return false;
  }

  juce::WavAudioFormat wav_format;
  std::unique_ptr<juce::AudioFormatWriter> writer(
      wav_format.createWriterFor(stream.release(), static_cast<double>(sample_rate_hz),
                                 static_cast<unsigned int>(num_channels), 16, {}, 0));
  if (!writer) {
    error_text = "Failed to create WAV writer.";
    return false;
  }
  if (!writer->writeFromAudioSampleBuffer(audio_buffer, 0, samples_per_channel)) {
    error_text = "Failed to write WAV file.";
    return false;
  }

  output_path = target.getFullPathName();
  return true;
}

bool AudioRecorder::IsRecording() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return recording_active_;
}

void AudioRecorder::ConsumePlayback(
    std::span<const std::int16_t> frame,
    const tempolink::audio::AudioPlaybackConfig& config) {
  std::unique_lock<std::mutex> lock(mutex_, std::try_to_lock);
  if (!lock.owns_lock() || !recording_active_ || frame.empty()) {
    return;
  }

  if (sample_rate_hz_ == 0 || channels_ == 0) {
    sample_rate_hz_ = config.sample_rate_hz;
    channels_ = config.channels;
  }
  if (sample_rate_hz_ != config.sample_rate_hz || channels_ != config.channels) {
    return;
  }

  samples_.insert(samples_.end(), frame.begin(), frame.end());
}

}  // namespace tempolink::juceapp::bridge
