#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace tempolink::juceapp::style {

enum class ThemeMode {
    Dark,
    Light,
    System
};

class ThemeManager : private juce::Timer, 
                    private juce::DeletedAtShutdown {
public:
    class Listener {
    public:
        virtual ~Listener();
        virtual void themeChanged() = 0;
    };

    static ThemeManager& getInstance();
    ~ThemeManager() override;

    void setTheme(ThemeMode mode);
    ThemeMode getThemeMode() const { return current_mode_; }
    bool isDark() const { return is_dark_actual_; }

    void addListener(Listener* listener) {
        listeners_.add(listener);
    }

    void removeListener(Listener* listener) {
        listeners_.remove(listener);
    }

private:
    ThemeManager();
    void timerCallback() override;
    void updateActualTheme();
    void notifyListeners();

    ThemeMode current_mode_ = ThemeMode::Dark;
    bool is_dark_actual_ = true;
    juce::ListenerList<Listener> listeners_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ThemeManager)
};

} // namespace tempolink::juceapp::style
