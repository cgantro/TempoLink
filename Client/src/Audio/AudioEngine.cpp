#include <tempolink/audio/AudioEngine.h>

#include <algorithm>
#include <cmath>

AudioEngine::AudioEngine()
{
    deviceManager.initialise(2, 2, nullptr, true);
}

AudioEngine::~AudioEngine()
{
    stop();
}

void AudioEngine::start()
{
    if (running)
        return;

    deviceManager.addAudioCallback(this);
    running = true;
}

void AudioEngine::stop()
{
    if (!running)
        return;

    deviceManager.removeAudioCallback(this);
    running = false;
}

juce::StringArray AudioEngine::getAvailableInputDevices()
{
    return getDeviceNames(true);
}

juce::StringArray AudioEngine::getAvailableOutputDevices()
{
    return getDeviceNames(false);
}

juce::String AudioEngine::getCurrentInputDevice() const
{
    return deviceManager.getAudioDeviceSetup().inputDeviceName;
}

juce::String AudioEngine::getCurrentOutputDevice() const
{
    return deviceManager.getAudioDeviceSetup().outputDeviceName;
}

bool AudioEngine::setInputDevice(const juce::String& deviceName)
{
    auto setup = deviceManager.getAudioDeviceSetup();
    setup.inputDeviceName = deviceName;
    return deviceManager.setAudioDeviceSetup(setup, true).isEmpty();
}

bool AudioEngine::setOutputDevice(const juce::String& deviceName)
{
    auto setup = deviceManager.getAudioDeviceSetup();
    setup.outputDeviceName = deviceName;
    return deviceManager.setAudioDeviceSetup(setup, true).isEmpty();
}

double AudioEngine::getEstimatedLatencyMs() const
{
    if (auto* device = deviceManager.getCurrentAudioDevice())
    {
        const auto sampleRate = device->getCurrentSampleRate();
        const auto totalLatencySamples = device->getInputLatencyInSamples()
                                       + device->getOutputLatencyInSamples()
                                       + device->getCurrentBufferSizeSamples();

        if (sampleRate > 0.0)
            return static_cast<double>(totalLatencySamples) * 1000.0 / sampleRate;
    }

    const auto setup = deviceManager.getAudioDeviceSetup();
    if (setup.sampleRate > 0.0)
        return static_cast<double>(setup.bufferSize) * 1000.0 / setup.sampleRate;

    return 0.0;
}

float AudioEngine::getInputLevel() const
{
    return inputLevel.load();
}

void AudioEngine::audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                                   int numInputChannels,
                                                   float* const* outputChannelData,
                                                   int numOutputChannels,
                                                   int numSamples,
                                                   const juce::AudioIODeviceCallbackContext&)
{
    auto peak = 0.0f;

    for (auto channel = 0; channel < numInputChannels; ++channel)
    {
        if (auto* input = inputChannelData[channel])
        {
            for (auto sample = 0; sample < numSamples; ++sample)
                peak = std::max(peak, std::abs(input[sample]));
        }
    }

    inputLevel.store(peak);

    for (auto channel = 0; channel < numOutputChannels; ++channel)
    {
        if (auto* output = outputChannelData[channel])
            std::fill(output, output + numSamples, 0.0f);
    }
}

void AudioEngine::audioDeviceAboutToStart(juce::AudioIODevice*)
{
    inputLevel.store(0.0f);
}

void AudioEngine::audioDeviceStopped()
{
    inputLevel.store(0.0f);
}

juce::StringArray AudioEngine::getDeviceNames(bool wantInputNames)
{
    juce::StringArray names;
    const auto& types = deviceManager.getAvailableDeviceTypes();

    for (auto* type : types)
        names.addArray(type->getDeviceNames(wantInputNames));

    names.removeDuplicates(false);
    return names;
}
