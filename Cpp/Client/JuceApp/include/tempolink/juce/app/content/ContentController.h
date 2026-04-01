#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include "tempolink/client/ClientSession.h"
#include "tempolink/juce/network/http/common/DocumentContentService.h"
#include "tempolink/juce/network/http/news/NewsApiClient.h"
#include "tempolink/juce/network/http/user/ProfileApiClient.h"
#include "tempolink/juce/network/http/user/UsersApiClient.h"
#include "tempolink/juce/ui/screens/ManualView.h"
#include "tempolink/juce/ui/screens/NewsView.h"
#include "tempolink/juce/ui/screens/ProfileView.h"
#include "tempolink/juce/ui/screens/QnaView.h"
#include "tempolink/juce/ui/screens/SettingsView.h"
#include "tempolink/juce/ui/screens/UsersView.h"

class ContentController {
 public:
  ContentController(
      UsersView& users_view, NewsView& news_view, ProfileView& profile_view,
      ManualView& manual_view, QnaView& qna_view, SettingsView& settings_view,
      UsersApiClient& users_api, NewsApiClient& news_api,
      ProfileApiClient& profile_api, DocumentContentService& document_service,
      tempolink::client::ClientSession& session,
      std::shared_ptr<std::atomic_bool> alive_flag,
      const std::string& control_plane_base_url, std::string& current_user_id,
      std::string& current_display_name, std::string& current_bio);

  void refreshUsers(const std::string& query = {});
  void refreshNews();
  void refreshProfile();
  void refreshManual();
  void refreshQna();
  void saveProfileSettings(const std::string& display_name,
                           const std::string& bio);
  void applySettingsAudioFormat(int sample_rate_hz, int buffer_samples);
  void runSettingsInputTest();
  void runSettingsOutputTest();
  void refreshSettingsView();

 private:
  UsersView& users_view_;
  NewsView& news_view_;
  ProfileView& profile_view_;
  ManualView& manual_view_;
  QnaView& qna_view_;
  SettingsView& settings_view_;

  UsersApiClient& users_api_;
  NewsApiClient& news_api_;
  ProfileApiClient& profile_api_;
  DocumentContentService& document_service_;
  tempolink::client::ClientSession& session_;

  std::shared_ptr<std::atomic_bool> alive_flag_;
  const std::string& control_plane_base_url_;
  std::string& current_user_id_;
  std::string& current_display_name_;
  std::string& current_bio_;
};
