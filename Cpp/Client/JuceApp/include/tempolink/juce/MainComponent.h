#pragma once

#include <memory>

#include <juce_gui_extra/juce_gui_extra.h>

#include "tempolink/juce/ui/LobbyView.h"
#include "tempolink/juce/ui/RoomEntrySettingsView.h"
#include "tempolink/juce/ui/SessionView.h"

class ClientAppPresenter;

class MainComponent final : public juce::Component, private juce::Timer {
 public:
  MainComponent();
  ~MainComponent() override;

  void resized() override;

 private:
  void timerCallback() override;

  LobbyView lobby_view_;
  RoomEntrySettingsView room_entry_view_;
  SessionView session_view_;
  std::unique_ptr<ClientAppPresenter> presenter_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
