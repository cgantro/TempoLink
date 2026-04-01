#include "tempolink/juce/style/ThemeManager.h"

namespace tempolink::juceapp::style {

ThemeManager::Listener::~Listener() = default;

ThemeManager& ThemeManager::getInstance() {
    static ThemeManager* instance = nullptr;
    if (instance == nullptr) {
        instance = new ThemeManager();
    }
    return *instance;
}

ThemeManager::~ThemeManager() {
    stopTimer();
}

ThemeManager::ThemeManager() {
    updateActualTheme();
    startTimer(1000); // Check system theme once a second if in System mode
}

void ThemeManager::timerCallback() {
    if (current_mode_ == ThemeMode::System) {
        updateActualTheme();
    }
}

void ThemeManager::setTheme(ThemeMode mode) {
    if (current_mode_ == mode) return;
    current_mode_ = mode;
    updateActualTheme();
}

void ThemeManager::updateActualTheme() {
    bool new_is_dark = true;
    if (current_mode_ == ThemeMode::Dark) {
        new_is_dark = true;
    } else if (current_mode_ == ThemeMode::Light) {
        new_is_dark = false;
    } else {
        new_is_dark = juce::Desktop::getInstance().isDarkModeActive();
    }

    if (new_is_dark != is_dark_actual_) {
        is_dark_actual_ = new_is_dark;
        notifyListeners();
    }
}

void ThemeManager::notifyListeners() {
    listeners_.call(&Listener::themeChanged);
}

} // namespace tempolink::juceapp::style
