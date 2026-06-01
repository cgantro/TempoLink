#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class NetworkStatusView : public juce::Component
{
public:
    NetworkStatusView();

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NetworkStatusView)
};
