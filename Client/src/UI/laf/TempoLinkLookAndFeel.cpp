#include <tempolink/ui/laf/TempoLinkLookAndFeel.h>

TempoLinkLookAndFeel::TempoLinkLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(kBackground));
    setColour(juce::TextButton::buttonColourId, juce::Colour(kSurface));
    setColour(juce::TextButton::buttonOnColourId, juce::Colour(kAccent));
    setColour(juce::TextButton::textColourOffId, juce::Colour(kText));
    setColour(juce::TextButton::textColourOnId, juce::Colours::black.withAlpha(0.86f));
    setColour(juce::TextEditor::backgroundColourId, juce::Colour(kSurface));
    setColour(juce::TextEditor::textColourId, juce::Colour(kText));
    setColour(juce::TextEditor::outlineColourId, juce::Colour(kText).withAlpha(0.16f));
    setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(kAccent));
    setColour(juce::Label::textColourId, juce::Colour(kText));
}
