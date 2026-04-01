#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "tempolink/juce/ui/models/UiModels.h"
#include "tempolink/juce/style/ThemeManager.h"

namespace tempolink::juceapp::style {

inline constexpr int kUiTimerHz = 30;
inline constexpr int kLobbyRefreshTicks = 150;
inline constexpr int kDefaultMetronomeBpm = 120;
inline constexpr float kDefaultMasterVolume = 1.0F;
inline constexpr float kMutedParticipantLevel = 0.03F;
inline constexpr float kAnimatedLevelMin = 0.05F;
inline constexpr float kAnimatedLevelMax = 0.95F;
inline constexpr float kAnimatedPhaseStep = 0.045F;
inline constexpr int kAnimatedCycle = 300;

inline constexpr int kLobbyPadding = 16;
inline constexpr int kLobbyHeaderHeight = 40;
inline constexpr int kLobbySearchWidth = 220;
inline constexpr int kLobbyGridPadding = 12;
inline constexpr int kLobbyCardHeight = 176;

inline constexpr int kSessionPadding = 14;
inline constexpr int kSessionLeftPanelWidth = 300;
inline constexpr int kParticipantRowHeight = 88;
inline constexpr int kParticipantRowPadding = 8;
inline constexpr int kParticipantAvatarSize = 26;

inline juce::Colour LobbyBackground() { 
    return ThemeManager::getInstance().isDark() ? juce::Colour::fromRGBA(11, 15, 26, 255) : juce::Colour::fromRGBA(249, 250, 251, 255); 
}
inline juce::Colour SessionBackground() { 
    return ThemeManager::getInstance().isDark() ? juce::Colour::fromRGBA(11, 15, 26, 255) : juce::Colour::fromRGBA(243, 244, 246, 255); 
}
inline juce::Colour PanelBackground() { 
    return ThemeManager::getInstance().isDark() ? juce::Colour::fromRGBA(17, 24, 39, 255) : juce::Colour::fromRGBA(255, 255, 255, 255); 
}
inline juce::Colour PanelBorder() { 
    return ThemeManager::getInstance().isDark() ? juce::Colour::fromRGBA(31, 41, 55, 255) : juce::Colour::fromRGBA(229, 231, 235, 255); 
}
inline juce::Colour CardBackground() { 
    return ThemeManager::getInstance().isDark() ? juce::Colour::fromRGBA(17, 24, 39, 255) : juce::Colour::fromRGBA(255, 255, 255, 255); 
}
inline juce::Colour CardBorder() { 
    return ThemeManager::getInstance().isDark() ? juce::Colour::fromRGBA(31, 41, 55, 255) : juce::Colour::fromRGBA(229, 231, 235, 255); 
}
inline juce::Colour ParticipantCardBackground() {
    return ThemeManager::getInstance().isDark() ? juce::Colour::fromRGBA(17, 24, 39, 255) : juce::Colour::fromRGBA(255, 255, 255, 255);
}
inline juce::Colour ParticipantAvatarSelf() {
    return juce::Colour::fromRGBA(59, 130, 246, 255); // Primary Blue stays the same
}
inline juce::Colour ParticipantAvatarPeer() {
    return juce::Colour::fromRGBA(167, 139, 250, 255); // Accent Purple stays the same
}
inline juce::Colour ConnectedText() { 
    return ThemeManager::getInstance().isDark() ? juce::Colour::fromRGBA(34, 197, 94, 255) : juce::Colour::fromRGBA(22, 163, 74, 255); 
}
inline juce::Colour SyncingText() { 
    return ThemeManager::getInstance().isDark() ? juce::Colour::fromRGBA(234, 179, 8, 255) : juce::Colour::fromRGBA(202, 138, 4, 255); 
}
inline juce::Colour TextPrimary() { 
    return ThemeManager::getInstance().isDark() ? juce::Colour::fromRGBA(229, 231, 235, 255) : juce::Colour::fromRGBA(17, 24, 39, 255); 
}
inline juce::Colour TextSecondary() { 
    return ThemeManager::getInstance().isDark() ? juce::Colour::fromRGBA(156, 163, 175, 255) : juce::Colour::fromRGBA(75, 85, 99, 255); 
}
inline juce::Colour TextInverted() { 
    return ThemeManager::getInstance().isDark() ? juce::Colours::white : juce::Colours::black; 
}
inline juce::Colour BorderStrong() { 
    return ThemeManager::getInstance().isDark() ? juce::Colour::fromRGBA(75, 85, 99, 255) : juce::Colour::fromRGBA(156, 163, 175, 255); 
}

inline juce::Colour PrimaryBlue() { return juce::Colour::fromRGBA(59, 130, 246, 255); }
inline juce::Colour AccentCyan() { return juce::Colour::fromRGBA(34, 211, 238, 255); }
inline juce::Colour HighlightPurple() { return juce::Colour::fromRGBA(167, 139, 250, 255); }

inline juce::Colour ConnectionBadgeBackground(ConnectionBadgeState state) {
  switch (state) {
    case ConnectionBadgeState::P2P:
      return juce::Colour::fromRGBA(34, 211, 238, 255); // Cyan for real-time
    case ConnectionBadgeState::Relay:
      return juce::Colour::fromRGBA(59, 130, 246, 255); // Primary Blue
    case ConnectionBadgeState::Reconnecting:
      return juce::Colour::fromRGBA(234, 179, 8, 255);
    case ConnectionBadgeState::Failed:
      return juce::Colour::fromRGBA(239, 68, 68, 255);
    case ConnectionBadgeState::Unknown:
    default:
      return juce::Colour::fromRGBA(75, 85, 99, 255);
  }
}

inline juce::String ConnectionBadgeText(ConnectionBadgeState state) {
  switch (state) {
    case ConnectionBadgeState::P2P:
      return "P2P";
    case ConnectionBadgeState::Relay:
      return "Relay";
    case ConnectionBadgeState::Reconnecting:
      return "Reconnecting";
    case ConnectionBadgeState::Failed:
      return "Failed";
    case ConnectionBadgeState::Unknown:
    default:
      return "Unknown";
  }
}

}  // namespace tempolink::juceapp::style
