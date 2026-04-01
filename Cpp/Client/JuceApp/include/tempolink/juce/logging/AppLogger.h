#pragma once

#include <juce_core/juce_core.h>

namespace tempolink::juceapp::logging {

void Initialize(const juce::String& app_name, const juce::String& app_version);
void Shutdown();

void Info(const juce::String& message);
void Warn(const juce::String& message);
void Error(const juce::String& message);

juce::File LogDirectory();
juce::String CurrentLogFilePath();

}  // namespace tempolink::juceapp::logging

