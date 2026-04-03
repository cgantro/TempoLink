#pragma once

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

namespace tempolink::juceapp::di {

/// Holds references to all views. Passed as a single object
/// instead of 11 constructor arguments.
struct ViewRegistry {
  LoginView& login_view;
  LobbyView& lobby_view;
  MyRoomsView& my_rooms_view;
  RoomEntrySettingsView& room_entry_view;
  SessionView& session_view;
  ProfileView& profile_view;
  UsersView& users_view;
  NewsView& news_view;
  ManualView& manual_view;
  QnaView& qna_view;
  SettingsView& settings_view;
};

}  // namespace tempolink::juceapp::di
