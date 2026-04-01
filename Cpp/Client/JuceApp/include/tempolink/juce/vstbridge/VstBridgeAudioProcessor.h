#pragma once

#include <cstdint>
#include <mutex>
#include <span>
#include <string>
#include <vector>

#include <juce_audio_processors/juce_audio_processors.h>

#include "tempolink/net/UdpSocket.h"

namespace tempolink::juceapp::vstbridge {

struct BridgeRuntimeConfig {
  std::string host;
  std::uint16_t client_listen_port = 0;
  std::uint16_t plugin_listen_port = 0;
};

class VstBridgeAudioProcessor final : public juce::AudioProcessor {
 public:
  VstBridgeAudioProcessor();
  ~VstBridgeAudioProcessor() override;

  void prepareToPlay(double sample_rate, int samples_per_block) override;
  void releaseResources() override;
  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
  void processBlock(juce::AudioBuffer<float>& buffer,
                    juce::MidiBuffer& midi_messages) override;

  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override;

  const juce::String getName() const override;
  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String& new_name) override;

  void getStateInformation(juce::MemoryBlock& dest_data) override;
  void setStateInformation(const void* data, int size_in_bytes) override;

 private:
  BridgeRuntimeConfig loadRuntimeConfig() const;
  bool ensureSocketsReady();
  bool sendDawInput(const juce::AudioBuffer<float>& buffer);
  bool receiveClientOutput(std::size_t expected_samples,
                           std::uint32_t sample_rate_hz, std::uint16_t channels,
                           std::vector<std::int16_t>& out_samples);
  void writeOutput(juce::AudioBuffer<float>& buffer,
                   std::span<const std::int16_t> interleaved) const;

  mutable std::mutex io_mutex_;
  tempolink::net::UdpSocket rx_socket_;
  tempolink::net::UdpSocket tx_socket_;
  BridgeRuntimeConfig bridge_config_;
  std::uint64_t tx_sequence_ = 0;
  std::uint32_t prepared_sample_rate_hz_ = 48000;
  int prepared_block_size_ = 0;
  std::vector<std::int16_t> interleaved_input_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VstBridgeAudioProcessor)
};

}  // namespace tempolink::juceapp::vstbridge
