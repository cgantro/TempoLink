#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class TempoLinkLookAndFeel : public juce::LookAndFeel_V4
{
public:
    TempoLinkLookAndFeel();

private:
    static constexpr juce::uint32 kBackground = 0xFF0A1430;
    static constexpr juce::uint32 kSurface    = 0xFF070F25;
    static constexpr juce::uint32 kAccent     = 0xFFCBA14C;
    static constexpr juce::uint32 kText       = 0xFFF1E8CF;
};
