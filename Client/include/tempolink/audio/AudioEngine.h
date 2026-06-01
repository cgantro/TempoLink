#pragma once
#include <atomic>
#include <juce_audio_devices/juce_audio_devices.h>

class AudioEngine : public juce::AudioIODeviceCallback
{
public:
    AudioEngine();
    ~AudioEngine() override;

    void start();
    void stop();

    juce::StringArray getAvailableInputDevices();
    juce::StringArray getAvailableOutputDevices();
    juce::String getCurrentInputDevice() const;
    juce::String getCurrentOutputDevice() const;
    bool setInputDevice(const juce::String& deviceName);
    bool setOutputDevice(const juce::String& deviceName);
    double getEstimatedLatencyMs() const;
    float getInputLevel() const;

    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                          int numInputChannels,
                                          float* const* outputChannelData,
                                          int numOutputChannels,
                                          int numSamples,
                                          const juce::AudioIODeviceCallbackContext&) override;
    void audioDeviceAboutToStart(juce::AudioIODevice*) override;
    void audioDeviceStopped() override;

private:
    juce::StringArray getDeviceNames(bool wantInputNames);

    juce::AudioDeviceManager deviceManager;
    std::atomic<float> inputLevel { 0.0f };
    bool running = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioEngine)
};
