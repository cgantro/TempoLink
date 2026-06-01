#include <tempolink/ui/MainComponent.h>

namespace
{
constexpr juce::uint32 kBackground = 0xFF0A1430;
constexpr juce::uint32 kInkFaint = 0x66F1E8CF;

juce::Colour colour(juce::uint32 argb)
{
    return juce::Colour(argb);
}
} // namespace

MainComponent::MainComponent()
    : authStartView(std::make_unique<tempolink::ui::AuthStartView>())
{
    setLookAndFeel(&lookAndFeel);
    setSize(1280, 820);

    authStartView->setAuthCompletedCallback([this]
    {
        prototypeStatus.setText(juce::String::fromUTF8("인증 프로토타입 완료 - 로비 연결은 다음 단계입니다."), juce::dontSendNotification);
    });
    addAndMakeVisible(*authStartView);

    prototypeStatus.setFont(juce::Font(13.0f, juce::Font::plain));
    prototypeStatus.setColour(juce::Label::textColourId, colour(kInkFaint));
    prototypeStatus.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(prototypeStatus);
}

MainComponent::~MainComponent()
{
    setLookAndFeel(nullptr);
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(colour(kBackground));
}

void MainComponent::resized()
{
    auto area = getLocalBounds();
    authStartView->setBounds(area);

    auto statusArea = getLocalBounds().reduced(24, 18);
    prototypeStatus.setBounds(statusArea.removeFromBottom(22));
    prototypeStatus.toFront(false);
}
