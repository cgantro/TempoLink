#include "tempolink/juce/vstbridge/VstBridgeAudioProcessor.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "tempolink/bridge/UdpBridgeProtocol.h"

namespace tempolink::juceapp::vstbridge {
namespace {

std::string ReadEnvOrDefault(const char* key, std::string fallback) {
  if (const char* env_value = std::getenv(key);
      env_value != nullptr && env_value[0] != '\0') {
    return env_value;
  }
  return fallback;
}

std::uint16_t ReadEnvPortOrDefault(const char* key, std::uint16_t fallback) {
  if (const char* env_value = std::getenv(key);
      env_value != nullptr && env_value[0] != '\0') {
    try {
      const int parsed = std::stoi(env_value);
      if (parsed > 0 && parsed <= 65535) {
        return static_cast<std::uint16_t>(parsed);
      }
    } catch (...) {
    }
  }
  return fallback;
}

std::int16_t FloatToPcm16(float sample) {
  const float scaled = std::clamp(sample, -1.0F, 1.0F) * 32767.0F;
  return static_cast<std::int16_t>(std::lround(scaled));
}

float Pcm16ToFloat(std::int16_t sample) {
  return static_cast<float>(sample) / 32768.0F;
}

}  // namespace

VstBridgeAudioProcessor::VstBridgeAudioProcessor()
    : juce::AudioProcessor(juce::AudioProcessor::BusesProperties()
                               .withInput("Input", juce::AudioChannelSet::stereo(), true)
                               .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      bridge_config_(loadRuntimeConfig()) {}

VstBridgeAudioProcessor::~VstBridgeAudioProcessor() { releaseResources(); }

void VstBridgeAudioProcessor::prepareToPlay(double sample_rate, int samples_per_block) {
  prepared_sample_rate_hz_ =
      static_cast<std::uint32_t>(juce::jlimit(8000.0, 384000.0, sample_rate));
  prepared_block_size_ = std::max(0, samples_per_block);
  bridge_config_ = loadRuntimeConfig();
  interleaved_input_.clear();
}

void VstBridgeAudioProcessor::releaseResources() {
  std::scoped_lock lock(io_mutex_);
  rx_socket_.Close();
  tx_socket_.Close();
}

bool VstBridgeAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
  const auto& input = layouts.getMainInputChannelSet();
  const auto& output = layouts.getMainOutputChannelSet();
  if (input != output || output.isDisabled()) {
    return false;
  }
  return output == juce::AudioChannelSet::mono() ||
         output == juce::AudioChannelSet::stereo();
}

void VstBridgeAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer& midi_messages) {
  juce::ignoreUnused(midi_messages);
  juce::ScopedNoDenormals no_denormals;

  const int num_samples = buffer.getNumSamples();
  const int num_channels = buffer.getNumChannels();
  if (num_samples <= 0 || num_channels <= 0) {
    buffer.clear();
    return;
  }

  if (!sendDawInput(buffer)) {
    buffer.clear();
    return;
  }

  std::vector<std::int16_t> output_samples;
  const std::size_t expected_samples =
      static_cast<std::size_t>(num_channels) * static_cast<std::size_t>(num_samples);
  if (!receiveClientOutput(expected_samples, prepared_sample_rate_hz_,
                           static_cast<std::uint16_t>(num_channels), output_samples)) {
    buffer.clear();
    return;
  }

  writeOutput(buffer, std::span<const std::int16_t>(output_samples.data(),
                                                    output_samples.size()));
}

juce::AudioProcessorEditor* VstBridgeAudioProcessor::createEditor() {
  return new juce::GenericAudioProcessorEditor(*this);
}

bool VstBridgeAudioProcessor::hasEditor() const { return true; }

const juce::String VstBridgeAudioProcessor::getName() const {
  return "TempoLink VST3 Bridge";
}

bool VstBridgeAudioProcessor::acceptsMidi() const { return false; }

bool VstBridgeAudioProcessor::producesMidi() const { return false; }

bool VstBridgeAudioProcessor::isMidiEffect() const { return false; }

double VstBridgeAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int VstBridgeAudioProcessor::getNumPrograms() { return 1; }

int VstBridgeAudioProcessor::getCurrentProgram() { return 0; }

void VstBridgeAudioProcessor::setCurrentProgram(int index) { juce::ignoreUnused(index); }

const juce::String VstBridgeAudioProcessor::getProgramName(int index) {
  juce::ignoreUnused(index);
  return "Default";
}

void VstBridgeAudioProcessor::changeProgramName(int index,
                                                const juce::String& new_name) {
  juce::ignoreUnused(index, new_name);
}

void VstBridgeAudioProcessor::getStateInformation(juce::MemoryBlock& dest_data) {
  juce::MemoryOutputStream out(dest_data, false);
  out.writeString(juce::String(bridge_config_.host));
  out.writeInt(static_cast<int>(bridge_config_.client_listen_port));
  out.writeInt(static_cast<int>(bridge_config_.plugin_listen_port));
}

void VstBridgeAudioProcessor::setStateInformation(const void* data, int size_in_bytes) {
  if (data == nullptr || size_in_bytes <= 0) {
    return;
  }
  juce::MemoryInputStream in(data, static_cast<size_t>(size_in_bytes), false);
  bridge_config_.host = in.readString().toStdString();
  bridge_config_.client_listen_port = static_cast<std::uint16_t>(
      juce::jlimit(1, 65535, in.readInt()));
  bridge_config_.plugin_listen_port = static_cast<std::uint16_t>(
      juce::jlimit(1, 65535, in.readInt()));
}

BridgeRuntimeConfig VstBridgeAudioProcessor::loadRuntimeConfig() const {
  BridgeRuntimeConfig config;
  config.host = ReadEnvOrDefault("TEMPOLINK_BRIDGE_HOST",
                                 tempolink::bridge::kDefaultBridgeHost);
  config.client_listen_port =
      ReadEnvPortOrDefault("TEMPOLINK_BRIDGE_CLIENT_LISTEN_PORT",
                           tempolink::bridge::kDefaultClientListenPort);
  config.plugin_listen_port =
      ReadEnvPortOrDefault("TEMPOLINK_BRIDGE_PLUGIN_LISTEN_PORT",
                           tempolink::bridge::kDefaultPluginListenPort);
  return config;
}

bool VstBridgeAudioProcessor::ensureSocketsReady() {
  std::scoped_lock lock(io_mutex_);
  if (rx_socket_.IsOpen() && tx_socket_.IsOpen()) {
    return true;
  }

  rx_socket_.Close();
  tx_socket_.Close();
  if (!rx_socket_.Open()) {
    return false;
  }
  if (!rx_socket_.Bind(bridge_config_.plugin_listen_port)) {
    rx_socket_.Close();
    return false;
  }
  if (!rx_socket_.SetNonBlocking(true)) {
    rx_socket_.Close();
    return false;
  }

  if (!tx_socket_.Open()) {
    rx_socket_.Close();
    return false;
  }
  if (!tx_socket_.SetNonBlocking(true)) {
    tx_socket_.Close();
    rx_socket_.Close();
    return false;
  }
  return true;
}

bool VstBridgeAudioProcessor::sendDawInput(const juce::AudioBuffer<float>& buffer) {
  if (!ensureSocketsReady()) {
    return false;
  }

  const int num_channels = buffer.getNumChannels();
  const int num_samples = buffer.getNumSamples();
  if (num_channels <= 0 || num_samples <= 0) {
    return false;
  }

  const std::size_t sample_count =
      static_cast<std::size_t>(num_channels) * static_cast<std::size_t>(num_samples);
  interleaved_input_.resize(sample_count);

  for (int frame = 0; frame < num_samples; ++frame) {
    for (int channel = 0; channel < num_channels; ++channel) {
      const std::size_t idx = static_cast<std::size_t>(frame) *
                                  static_cast<std::size_t>(num_channels) +
                              static_cast<std::size_t>(channel);
      interleaved_input_[idx] =
          FloatToPcm16(buffer.getSample(channel, frame));
    }
  }

  tempolink::bridge::UdpBridgePacketHeader header;
  header.type = tempolink::bridge::UdpBridgePacketType::kDawToClient;
  header.sample_rate_hz = prepared_sample_rate_hz_;
  header.channels = static_cast<std::uint16_t>(num_channels);
  header.frames = static_cast<std::uint16_t>(num_samples);
  header.sequence = tx_sequence_++;

  const auto packet = tempolink::bridge::SerializeUdpBridgePacket(
      header, std::span<const std::int16_t>(interleaved_input_.data(),
                                            interleaved_input_.size()));

  std::scoped_lock lock(io_mutex_);
  return tx_socket_.SendTo(
      std::span<const std::byte>(packet.data(), packet.size()), bridge_config_.host,
      bridge_config_.client_listen_port);
}

bool VstBridgeAudioProcessor::receiveClientOutput(
    std::size_t expected_samples, std::uint32_t sample_rate_hz, std::uint16_t channels,
    std::vector<std::int16_t>& out_samples) {
  (void)channels;
  if (!ensureSocketsReady()) {
    return false;
  }

  const std::size_t max_datagram_bytes =
      expected_samples * sizeof(std::int16_t) + 64U;
  const std::size_t read_capacity =
      std::min(max_datagram_bytes, rx_datagram_buffer_.size());
  if (read_capacity == 0U) {
    return false;
  }

  std::size_t latest_bytes = 0;
  {
    std::scoped_lock lock(io_mutex_);
    while (true) {
      const auto receive_result =
          rx_socket_.ReceiveFrom(std::span<std::byte>(rx_datagram_buffer_.data(),
                                                      read_capacity));
      if (receive_result.status == tempolink::net::SocketStatus::WouldBlock) {
        break;
      }
      if (receive_result.status != tempolink::net::SocketStatus::Success) {
        break;
      }
      latest_bytes = receive_result.bytes_read;
    }
  }

  if (latest_bytes == 0U) {
    return false;
  }

  tempolink::bridge::ParsedUdpBridgePacket packet;
  if (!tempolink::bridge::ParseUdpBridgePacket(
          std::span<const std::byte>(rx_datagram_buffer_.data(), latest_bytes), packet)) {
    return false;
  }
  if (packet.header.type != tempolink::bridge::UdpBridgePacketType::kClientToDaw) {
    return false;
  }
  if (packet.header.sample_rate_hz != sample_rate_hz) {
    return false;
  }
  if (packet.samples.size() < expected_samples) {
    return false;
  }

  out_samples.assign(packet.samples.begin(),
                     packet.samples.begin() + expected_samples);
  return true;
}

void VstBridgeAudioProcessor::writeOutput(
    juce::AudioBuffer<float>& buffer, std::span<const std::int16_t> interleaved) const {
  const int num_channels = buffer.getNumChannels();
  const int num_samples = buffer.getNumSamples();
  if (num_channels <= 0 || num_samples <= 0) {
    return;
  }

  for (int frame = 0; frame < num_samples; ++frame) {
    for (int channel = 0; channel < num_channels; ++channel) {
      const std::size_t idx = static_cast<std::size_t>(frame) *
                                  static_cast<std::size_t>(num_channels) +
                              static_cast<std::size_t>(channel);
      if (idx >= interleaved.size()) {
        buffer.setSample(channel, frame, 0.0F);
        continue;
      }
      buffer.setSample(channel, frame, Pcm16ToFloat(interleaved[idx]));
    }
  }
}

}  // namespace tempolink::juceapp::vstbridge

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new tempolink::juceapp::vstbridge::VstBridgeAudioProcessor();
}
