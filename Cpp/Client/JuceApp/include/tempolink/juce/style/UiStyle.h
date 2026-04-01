#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "tempolink/juce/ui/models/UiModels.h"

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

inline juce::Colour LobbyBackground() { return juce::Colour::fromRGB(237, 241, 254); }
inline juce::Colour SessionBackground() { return juce::Colour::fromRGB(241, 244, 255); }
inline juce::Colour PanelBackground() { return juce::Colour::fromRGB(250, 251, 255); }
inline juce::Colour PanelBorder() { return juce::Colour::fromRGB(215, 222, 248); }
inline juce::Colour CardBackground() { return juce::Colour::fromRGB(244, 246, 255); }
inline juce::Colour CardBorder() { return juce::Colour::fromRGB(214, 220, 245); }
inline juce::Colour ParticipantCardBackground() {
  return juce::Colour::fromRGB(248, 249, 255);
}
inline juce::Colour ParticipantAvatarSelf() {
  return juce::Colour::fromRGB(83, 112, 255);
}
inline juce::Colour ParticipantAvatarPeer() {
  return juce::Colour::fromRGB(123, 149, 255);
}
inline juce::Colour ConnectedText() { return juce::Colour::fromRGB(20, 145, 68); }
inline juce::Colour SyncingText() { return juce::Colour::fromRGB(196, 124, 26); }
inline juce::Colour TextPrimary() { return juce::Colour::fromRGB(20, 29, 51); }
inline juce::Colour TextSecondary() { return juce::Colour::fromRGB(74, 86, 120); }
inline juce::Colour TextInverted() { return juce::Colours::white; }
inline juce::Colour BorderStrong() { return juce::Colour::fromRGB(175, 188, 230); }

inline juce::Colour ConnectionBadgeBackground(ConnectionBadgeState state) {
  switch (state) {
    case ConnectionBadgeState::P2P:
      return juce::Colour::fromRGB(32, 156, 86);
    case ConnectionBadgeState::Relay:
      return juce::Colour::fromRGB(66, 100, 214);
    case ConnectionBadgeState::Reconnecting:
      return juce::Colour::fromRGB(231, 158, 46);
    case ConnectionBadgeState::Failed:
      return juce::Colour::fromRGB(214, 73, 73);
    case ConnectionBadgeState::Unknown:
    default:
      return juce::Colour::fromRGB(124, 136, 168);
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
