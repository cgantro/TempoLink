#pragma once

#include <memory>

#include <juce_gui_extra/juce_gui_extra.h>
#include "tempolink/juce/di/DependencyContainer.h"
#include "tempolink/juce/di/ViewRegistry.h"
#include "tempolink/juce/style/ThemeManager.h"

#include "tempolink/juce/ui/screens/LobbyView.h"
#include "tempolink/juce/ui/screens/LoginView.h"
#include "tempolink/juce/ui/screens/MyRoomsView.h"
#include "tempolink/juce/ui/screens/NewsView.h"
#include "tempolink/juce/ui/screens/ManualView.h"
#include "tempolink/juce/ui/screens/ProfileView.h"
#include "tempolink/juce/ui/screens/QnaView.h"
#include "tempolink/juce/ui/screens/RoomEntrySettingsView.h"
#include "tempolink/juce/ui/screens/SessionView.h"
#include "tempolink/juce/ui/screens/SettingsView.h"
#include "tempolink/juce/ui/screens/UsersView.h"

class ClientAppPresenter;
namespace tempolink::juceapp::bridge {
class UdpAudioBridgePort;
class AudioSessionService;
}

class MainComponent final : public juce::Component, 
                           private juce::Timer,
                           public tempolink::juceapp::style::ThemeManager::Listener {
 public:
  MainComponent();
  ~MainComponent() override;

  void themeChanged() override;

  void resized() override;

 private:
  void timerCallback() override;

  LobbyView lobby_view_;
  LoginView login_view_;
  MyRoomsView my_rooms_view_;
  RoomEntrySettingsView room_entry_view_;
  SessionView session_view_;
  ProfileView profile_view_;
  UsersView users_view_;
  NewsView news_view_;
  ManualView manual_view_;
  QnaView qna_view_;
  SettingsView settings_view_;
  tempolink::juceapp::di::DependencyContainer deps_;
  std::unique_ptr<ClientAppPresenter> presenter_;
  std::shared_ptr<tempolink::juceapp::bridge::UdpAudioBridgePort> udp_audio_bridge_;
  std::shared_ptr<tempolink::juceapp::bridge::AudioSessionService>
      audio_session_service_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
