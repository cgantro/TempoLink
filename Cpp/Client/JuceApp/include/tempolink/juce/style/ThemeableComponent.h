#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "tempolink/juce/style/ThemeManager.h"

namespace tempolink::juceapp::style {

/**
 * Base class for components that need to respond to theme changes.
 * Automatically registers/unregisters with ThemeManager and provides
 * a virtual updateTheme() method.
 */
class ThemeableComponent : public juce::Component,
                           public ThemeManager::Listener {
public:
    ThemeableComponent() {
        ThemeManager::getInstance().addListener(this);
    }

    virtual ~ThemeableComponent() override {
        ThemeManager::getInstance().removeListener(this);
    }

    void themeChanged() override {
        updateTheme();
    }

    /**
     * Override this to update your component's colors/styles.
     * This is called automatically when the theme changes.
     */
    virtual void updateTheme() = 0;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ThemeableComponent)
};

} // namespace tempolink::juceapp::style
