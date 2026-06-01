#pragma once
#include <memory>
#include <juce_gui_basics/juce_gui_basics.h>
#include "laf/TempoLinkLookAndFeel.h"
#include "views/AuthStartView.h"

class MainComponent : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    TempoLinkLookAndFeel lookAndFeel;
    std::unique_ptr<tempolink::ui::AuthStartView> authStartView;
    juce::Label prototypeStatus;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
