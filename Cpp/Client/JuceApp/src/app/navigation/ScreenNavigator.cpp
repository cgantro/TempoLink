#include "tempolink/juce/app/navigation/ScreenNavigator.h"

namespace tempolink::juceapp::app {

ScreenNavigator::ScreenNavigator(LoginView& login_view, LobbyView& lobby_view,
                                 MyRoomsView& my_rooms_view,
                                 RoomEntrySettingsView& room_entry_view,
                                 SessionView& session_view,
                                 ProfileView& profile_view,
                                 UsersView& users_view, NewsView& news_view,
                                 ManualView& manual_view, QnaView& qna_view,
                                 SettingsView& settings_view)
    : login_view_(login_view),
      lobby_view_(lobby_view),
      my_rooms_view_(my_rooms_view),
      room_entry_view_(room_entry_view),
      session_view_(session_view),
      profile_view_(profile_view),
      users_view_(users_view),
      news_view_(news_view),
      manual_view_(manual_view),
      qna_view_(qna_view),
      settings_view_(settings_view) {}

void ScreenNavigator::SwitchTo(ScreenMode mode) {
  current_mode_ = mode;
  const bool show_login = current_mode_ == ScreenMode::Login;
  const bool show_lobby = current_mode_ == ScreenMode::Lobby;
  const bool show_my_rooms = current_mode_ == ScreenMode::MyRooms;
  const bool show_room_entry = current_mode_ == ScreenMode::RoomEntry;
  const bool show_session = current_mode_ == ScreenMode::Session;
  const bool show_profile = current_mode_ == ScreenMode::Profile;
  const bool show_users = current_mode_ == ScreenMode::Users;
  const bool show_news = current_mode_ == ScreenMode::News;
  const bool show_manual = current_mode_ == ScreenMode::Manual;
  const bool show_qna = current_mode_ == ScreenMode::Qna;
  const bool show_settings = current_mode_ == ScreenMode::Settings;

  login_view_.setVisible(show_login);
  lobby_view_.setVisible(show_lobby);
  my_rooms_view_.setVisible(show_my_rooms);
  room_entry_view_.setVisible(show_room_entry);
  session_view_.setVisible(show_session);
  profile_view_.setVisible(show_profile);
  users_view_.setVisible(show_users);
  news_view_.setVisible(show_news);
  manual_view_.setVisible(show_manual);
  qna_view_.setVisible(show_qna);
  settings_view_.setVisible(show_settings);
}

ScreenMode ScreenNavigator::CurrentMode() const { return current_mode_; }

}  // namespace tempolink::juceapp::app
