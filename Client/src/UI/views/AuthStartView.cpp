#include <tempolink/ui/views/AuthStartView.h>

#include <cmath>
#include <utility>

namespace tempolink::ui
{
namespace
{
constexpr juce::uint32 kBackground = 0xFF0A1430;
constexpr juce::uint32 kDeepBackground = 0xFF070F25;
constexpr juce::uint32 kPanel = 0xFF0E1C40;
constexpr juce::uint32 kRaised = 0xFF15295A;
constexpr juce::uint32 kInk = 0xFFF1E8CF;
constexpr juce::uint32 kInkSoft = 0xB0F1E8CF;
constexpr juce::uint32 kInkFaint = 0x66F1E8CF;
constexpr juce::uint32 kLine = 0x2AF1E8CF;
constexpr juce::uint32 kLineStrong = 0x4DF1E8CF;
constexpr juce::uint32 kGold = 0xFFCBA14C;
constexpr juce::uint32 kGoldDeep = 0xFFA07E34;
constexpr juce::uint32 kSteel = 0xFF84A4D6;

juce::Colour colour(juce::uint32 argb)
{
    return juce::Colour(argb);
}

void styleLabel(juce::Label& label, float height, juce::Colour text, bool bold = false)
{
    label.setFont(juce::Font(height, bold ? juce::Font::bold : juce::Font::plain));
    label.setColour(juce::Label::textColourId, text);
    label.setJustificationType(juce::Justification::centredLeft);
}

void styleEditor(juce::TextEditor& editor)
{
    editor.setColour(juce::TextEditor::backgroundColourId, colour(kDeepBackground));
    editor.setColour(juce::TextEditor::outlineColourId, colour(kLine));
    editor.setColour(juce::TextEditor::focusedOutlineColourId, colour(kGold));
    editor.setColour(juce::TextEditor::textColourId, colour(kInk));
    editor.setColour(juce::TextEditor::highlightColourId, colour(kGold).withAlpha(0.28f));
    editor.setColour(juce::CaretComponent::caretColourId, colour(kGold));
}

void stylePrimaryButton(juce::TextButton& button)
{
    button.setColour(juce::TextButton::buttonColourId, colour(kGold));
    button.setColour(juce::TextButton::buttonOnColourId, colour(kGoldDeep));
    button.setColour(juce::TextButton::textColourOffId, juce::Colours::black.withAlpha(0.86f));
    button.setColour(juce::TextButton::textColourOnId, juce::Colours::black.withAlpha(0.86f));
}
} // namespace

class AuthStartView::AuthHeroPanel final : public juce::Component
{
public:
    AuthHeroPanel()
    {
        styleLabel(eyebrow, 14.0f, colour(kGold), true);
        eyebrow.setText("TempoLink", juce::dontSendNotification);
        addAndMakeVisible(eyebrow);

        styleLabel(brandTempo, 88.0f, colour(kInk), true);
        brandTempo.setText("Tempo", juce::dontSendNotification);
        addAndMakeVisible(brandTempo);

        styleLabel(brandLink, 88.0f, colour(kGold), true);
        brandLink.setText("Link", juce::dontSendNotification);
        addAndMakeVisible(brandLink);

        styleLabel(tagline, 24.0f, colour(kInkSoft));
        tagline.setText(juce::String::fromUTF8("함께 연주할 준비가 끝났습니다."), juce::dontSendNotification);
        addAndMakeVisible(tagline);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(colour(kBackground));

        auto bounds = getLocalBounds().toFloat();
        g.setColour(colour(kLine));
        g.drawLine(bounds.getRight() - 1.0f, 0.0f, bounds.getRight() - 1.0f, bounds.getBottom(), 1.0f);

        drawStars(g);
        drawWaveform(g);
        drawMoon(g);

        g.setColour(colour(kLineStrong));
        const auto ruleY = bounds.getBottom() - 74.0f;
        g.drawLine(56.0f, ruleY, bounds.getRight() - 56.0f, ruleY, 1.0f);
        g.setColour(colour(kLine));
        g.drawLine(56.0f, ruleY + 4.0f, bounds.getRight() - 56.0f, ruleY + 4.0f, 1.0f);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(56, 54);
        eyebrow.setBounds(area.removeFromTop(26));

        auto brandArea = getLocalBounds().reduced(56, 0);
        const auto brandTop = juce::roundToInt(static_cast<float>(getHeight()) * 0.47f);
        brandArea.removeFromTop(brandTop);

        auto brandLine = brandArea.removeFromTop(104);
        brandTempo.setBounds(brandLine.removeFromLeft(360));
        brandLink.setBounds(brandLine.removeFromLeft(220));
        tagline.setBounds(brandArea.removeFromTop(42));
    }

private:
    void drawStars(juce::Graphics& g) const
    {
        g.setColour(colour(kGold).withAlpha(0.72f));
        drawAsterisk(g, { 46.0f, 34.0f }, 8.0f);
        drawAsterisk(g, { static_cast<float>(getWidth()) - 42.0f, static_cast<float>(getHeight()) - 32.0f }, 8.0f);
    }

    static void drawAsterisk(juce::Graphics& g, juce::Point<float> centre, float radius)
    {
        for (auto i = 0; i < 3; ++i)
        {
            auto angle = juce::MathConstants<float>::pi * static_cast<float>(i) / 3.0f;
            auto delta = juce::Point<float>(std::cos(angle) * radius, std::sin(angle) * radius);
            g.drawLine({ centre - delta, centre + delta }, 1.2f);
        }
    }

    void drawWaveform(juce::Graphics& g) const
    {
        auto wave = juce::Rectangle<float>(56.0f, 112.0f, static_cast<float>(getWidth()) - 112.0f, 52.0f);
        g.setColour(colour(kSteel));

        constexpr int bars = 48;
        const auto gap = 4.0f;
        const auto barWidth = (wave.getWidth() - gap * static_cast<float>(bars - 1)) / static_cast<float>(bars);

        for (auto i = 0; i < bars; ++i)
        {
            const auto phase = static_cast<float>(i) * 0.58f + 1.7f;
            const auto peak = 0.28f + std::abs(std::sin(phase)) * 0.68f;
            const auto height = juce::jmax(8.0f, wave.getHeight() * peak);
            const auto x = wave.getX() + static_cast<float>(i) * (barWidth + gap);
            const auto y = wave.getCentreY() - height * 0.5f;
            g.fillRoundedRectangle(x, y, barWidth, height, 2.0f);
        }
    }

    void drawMoon(juce::Graphics& g) const
    {
        auto moon = juce::Rectangle<float>(static_cast<float>(getWidth()) - 260.0f, 214.0f, 190.0f, 190.0f);

        juce::ColourGradient gradient(colour(kRaised), moon.getX() + 66.0f, moon.getY() + 54.0f,
                                      colour(kPanel), moon.getRight(), moon.getBottom(), true);
        g.setGradientFill(gradient);
        g.fillEllipse(moon);
        g.setColour(colour(kLine));
        g.drawEllipse(moon, 1.0f);

        g.setColour(colour(kBackground));
        g.fillEllipse(moon.withSizeKeepingCentre(150.0f, 150.0f).translated(26.0f, 6.0f));
    }

    juce::Label eyebrow;
    juce::Label brandTempo;
    juce::Label brandLink;
    juce::Label tagline;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AuthHeroPanel)
};

class AuthStartView::AuthPanel final : public juce::Component
{
public:
    explicit AuthPanel(std::function<void()> completed)
        : onCompleted(std::move(completed))
    {
        styleLabel(eyebrow, 14.0f, colour(kGold), true);
        eyebrow.setText("ACCOUNT", juce::dontSendNotification);
        addAndMakeVisible(eyebrow);

        styleLabel(title, 34.0f, colour(kInk), true);
        addAndMakeVisible(title);

        configureTab(loginTab, juce::String::fromUTF8("로그인"), AuthMode::Login);
        configureTab(signupTab, juce::String::fromUTF8("회원가입"), AuthMode::Signup);
        configureTab(googleTab, "Google", AuthMode::Google);

        configureField(nameLabel, nameEditor, juce::String::fromUTF8("이름"), juce::String::fromUTF8("예) 문라이트"));
        configureField(emailLabel, emailEditor, juce::String::fromUTF8("이메일"), "you@tempolink.app");
        configureField(passwordLabel, passwordEditor, juce::String::fromUTF8("비밀번호"), juce::String::fromUTF8("비밀번호"));
        passwordEditor.setPasswordCharacter('*');

        stylePrimaryButton(submitButton);
        submitButton.onClick = [this] { submit(); };
        addAndMakeVisible(submitButton);

        styleLabel(bodyText, 13.0f, colour(kInkFaint));
        bodyText.setText(juce::String::fromUTF8("브라우저 인증 연동 전 화면 흐름을 확인하는 프로토타입입니다."), juce::dontSendNotification);
        addChildComponent(bodyText);

        styleLabel(message, 12.0f, colour(kInkFaint));
        addAndMakeVisible(message);

        updateMode(AuthMode::Login);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(colour(kDeepBackground));
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(48, 54);
        area.removeFromTop(92);

        eyebrow.setBounds(48, 164, getWidth() - 96, 24);
        title.setBounds(48, 196, getWidth() - 96, 46);

        auto tabArea = juce::Rectangle<int>(48, 272, getWidth() - 96, 38);
        const auto tabWidth = tabArea.getWidth() / 3;
        loginTab.setBounds(tabArea.removeFromLeft(tabWidth));
        signupTab.setBounds(tabArea.removeFromLeft(tabWidth));
        googleTab.setBounds(tabArea);

        auto form = juce::Rectangle<int>(48, 344, getWidth() - 96, 260);
        if (mode != AuthMode::Google)
        {
            if (mode == AuthMode::Signup)
                layoutField(form, nameLabel, nameEditor);
            layoutField(form, emailLabel, emailEditor);
            layoutField(form, passwordLabel, passwordEditor);
        }

        if (mode == AuthMode::Google)
        {
            bodyText.setBounds(form.removeFromTop(56));
            form.removeFromTop(12);
        }

        submitButton.setBounds(form.removeFromTop(48));
        message.setBounds(48, submitButton.getBottom() + 18, getWidth() - 96, 24);
    }

private:
    void configureTab(juce::TextButton& button, const juce::String& text, AuthMode targetMode)
    {
        button.setButtonText(text);
        button.onClick = [this, targetMode] { updateMode(targetMode); };
        button.setColour(juce::TextButton::textColourOffId, colour(kInkFaint));
        button.setColour(juce::TextButton::textColourOnId, juce::Colours::black.withAlpha(0.86f));
        addAndMakeVisible(button);
    }

    void configureField(juce::Label& label, juce::TextEditor& editor, const juce::String& labelText, const juce::String& placeholder)
    {
        styleLabel(label, 12.0f, colour(kInkFaint), true);
        label.setText(labelText, juce::dontSendNotification);
        addAndMakeVisible(label);

        editor.setTextToShowWhenEmpty(placeholder, colour(kInkFaint));
        styleEditor(editor);
        addAndMakeVisible(editor);
    }

    static void layoutField(juce::Rectangle<int>& area, juce::Label& label, juce::TextEditor& editor)
    {
        label.setBounds(area.removeFromTop(22));
        editor.setBounds(area.removeFromTop(42));
        area.removeFromTop(16);
    }

    void updateMode(AuthMode nextMode)
    {
        mode = nextMode;
        message.setText({}, juce::dontSendNotification);

        const auto signup = mode == AuthMode::Signup;
        const auto google = mode == AuthMode::Google;

        title.setText(google ? juce::String::fromUTF8("Google로 계속하기")
                             : (signup ? juce::String::fromUTF8("회원가입") : juce::String::fromUTF8("로그인")),
                      juce::dontSendNotification);
        submitButton.setButtonText(google ? juce::String::fromUTF8("Google로 계속하기")
                                          : (signup ? juce::String::fromUTF8("계정 만들기") : juce::String::fromUTF8("로그인")));

        nameLabel.setVisible(signup);
        nameEditor.setVisible(signup);
        emailLabel.setVisible(!google);
        emailEditor.setVisible(!google);
        passwordLabel.setVisible(!google);
        passwordEditor.setVisible(!google);
        bodyText.setVisible(google);

        setActiveTab(loginTab, mode == AuthMode::Login);
        setActiveTab(signupTab, signup);
        setActiveTab(googleTab, google);
        resized();
    }

    static void setActiveTab(juce::TextButton& button, bool active)
    {
        button.setColour(juce::TextButton::buttonColourId, active ? colour(kGold) : colour(kDeepBackground));
        button.setColour(juce::TextButton::buttonOnColourId, active ? colour(kGoldDeep) : colour(kDeepBackground));
        button.setColour(juce::TextButton::textColourOffId, active ? juce::Colours::black.withAlpha(0.86f) : colour(kInkFaint));
    }

    void submit()
    {
        if (mode == AuthMode::Google)
        {
            message.setText(juce::String::fromUTF8("Google 로그인 프로토타입입니다."), juce::dontSendNotification);
            onCompleted();
            return;
        }

        const auto missingSignupName = mode == AuthMode::Signup && nameEditor.getText().trim().isEmpty();
        if (emailEditor.getText().trim().isEmpty() || passwordEditor.getText().trim().isEmpty() || missingSignupName)
        {
            message.setText(juce::String::fromUTF8("필수 정보를 입력해 주세요."), juce::dontSendNotification);
            return;
        }

        message.setText(mode == AuthMode::Signup ? juce::String::fromUTF8("회원가입 프로토타입이 완료되었습니다.")
                                                 : juce::String::fromUTF8("로그인 프로토타입이 완료되었습니다."),
                        juce::dontSendNotification);
        onCompleted();
    }

    AuthMode mode { AuthMode::Login };
    std::function<void()> onCompleted;

    juce::Label eyebrow;
    juce::Label title;
    juce::TextButton loginTab;
    juce::TextButton signupTab;
    juce::TextButton googleTab;
    juce::Label nameLabel;
    juce::TextEditor nameEditor;
    juce::Label emailLabel;
    juce::TextEditor emailEditor;
    juce::Label passwordLabel;
    juce::TextEditor passwordEditor;
    juce::Label bodyText;
    juce::TextButton submitButton;
    juce::Label message;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AuthPanel)
};

AuthStartView::AuthStartView()
    : heroPanel(std::make_unique<AuthHeroPanel>()),
      authPanel(std::make_unique<AuthPanel>([this] { handleAuthCompleted(); }))
{
    addAndMakeVisible(*heroPanel);
    addAndMakeVisible(*authPanel);
}

AuthStartView::~AuthStartView() = default;

void AuthStartView::setAuthCompletedCallback(AuthCompletedCallback callback)
{
    onAuthCompleted = std::move(callback);
}

void AuthStartView::paint(juce::Graphics& g)
{
    g.fillAll(colour(kBackground));
}

void AuthStartView::resized()
{
    auto area = getLocalBounds();
    const auto heroWidth = juce::roundToInt(static_cast<float>(area.getWidth()) * 0.575f);
    heroPanel->setBounds(area.removeFromLeft(heroWidth));
    authPanel->setBounds(area);
}

void AuthStartView::handleAuthCompleted()
{
    if (onAuthCompleted)
        onAuthCompleted();
}

} // namespace tempolink::ui
