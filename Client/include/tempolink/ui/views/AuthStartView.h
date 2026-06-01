#pragma once

#include <functional>
#include <memory>
#include <juce_gui_basics/juce_gui_basics.h>

namespace tempolink::ui
{

enum class AuthMode
{
    Login,
    Signup,
    Google
};

class AuthStartView final : public juce::Component
{
public:
    using AuthCompletedCallback = std::function<void()>;

    AuthStartView();
    ~AuthStartView() override;

    void setAuthCompletedCallback(AuthCompletedCallback callback);
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    class AuthHeroPanel;
    class AuthPanel;

    void handleAuthCompleted();

    std::unique_ptr<AuthHeroPanel> heroPanel;
    std::unique_ptr<AuthPanel> authPanel;
    AuthCompletedCallback onAuthCompleted;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AuthStartView)
};

} // namespace tempolink::ui
