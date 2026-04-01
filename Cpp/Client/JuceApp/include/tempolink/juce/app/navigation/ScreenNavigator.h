#pragma once

#include "tempolink/juce/app/navigation/ScreenMode.h"
#include "tempolink/juce/ui/screens/LobbyView.h"
#include "tempolink/juce/ui/screens/LoginView.h"
#include "tempolink/juce/ui/screens/ManualView.h"
#include "tempolink/juce/ui/screens/MyRoomsView.h"
#include "tempolink/juce/ui/screens/NewsView.h"
#include "tempolink/juce/ui/screens/ProfileView.h"
#include "tempolink/juce/ui/screens/QnaView.h"
#include "tempolink/juce/ui/screens/RoomEntrySettingsView.h"
#include "tempolink/juce/ui/screens/SessionView.h"
#include "tempolink/juce/ui/screens/SettingsView.h"
#include "tempolink/juce/ui/screens/UsersView.h"

namespace tempolink::juceapp::app {

class ScreenNavigator {
 public:
  ScreenNavigator(LoginView& login_view, LobbyView& lobby_view,
                  MyRoomsView& my_rooms_view,
                  RoomEntrySettingsView& room_entry_view,
                  SessionView& session_view, ProfileView& profile_view,
                  UsersView& users_view, NewsView& news_view,
                  ManualView& manual_view, QnaView& qna_view,
                  SettingsView& settings_view);

  void SwitchTo(ScreenMode mode);
  ScreenMode CurrentMode() const;

 private:
  LoginView& login_view_;
  LobbyView& lobby_view_;
  MyRoomsView& my_rooms_view_;
  RoomEntrySettingsView& room_entry_view_;
  SessionView& session_view_;
  ProfileView& profile_view_;
  UsersView& users_view_;
  NewsView& news_view_;
  ManualView& manual_view_;
  QnaView& qna_view_;
  SettingsView& settings_view_;
  ScreenMode current_mode_ = ScreenMode::Login;
};

}  // namespace tempolink::juceapp::app

