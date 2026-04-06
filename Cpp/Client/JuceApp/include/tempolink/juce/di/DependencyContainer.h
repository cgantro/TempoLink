#pragma once

#include <memory>
#include <string>

#include "tempolink/client/ClientSession.h"
#include "tempolink/juce/config/ClientEnvConfig.h"
#include "tempolink/juce/network/auth/AuthApiClient.h"
#include "tempolink/juce/network/auth/OAuthCallbackServer.h"
#include "tempolink/juce/network/http/common/DocumentContentService.h"
#include "tempolink/juce/network/http/news/NewsApiClient.h"
#include "tempolink/juce/network/http/room/RoomApiClient.h"
#include "tempolink/juce/network/http/user/ProfileApiClient.h"
#include "tempolink/juce/network/http/user/UsersApiClient.h"
#include "tempolink/juce/network/ice/IceConfigClient.h"
#include "tempolink/juce/network/signaling/SignalingClient.h"

namespace tempolink::juceapp::di {

/// Owns all external dependencies (API clients, networking, session).
/// Created once at app startup, injected into controllers.
/// Eliminates the need for each controller to know concrete types.
struct DependencyContainer {
  // --- Configuration (loaded from env) ---
  std::string control_plane_base_url =
      tempolink::juceapp::config::ClientEnvConfig::kDefaultControlPlaneBaseUrl;
  std::string control_plane_host =
      tempolink::juceapp::config::ClientEnvConfig::kDefaultControlPlaneHost;
  std::uint16_t control_plane_port =
      tempolink::juceapp::config::ClientEnvConfig::kDefaultControlPlanePort;
  std::string default_relay_host =
      tempolink::juceapp::config::ClientEnvConfig::kDefaultRelayHost;
  std::uint16_t default_relay_port =
      tempolink::juceapp::config::ClientEnvConfig::kDefaultRelayPort;

  // --- Core domain ---
  tempolink::client::ClientSession session;

  // --- API clients ---
  RoomApiClient room_api{control_plane_base_url};
  AuthApiClient auth_api{control_plane_base_url};
  IceConfigClient ice_client{control_plane_base_url};
  UsersApiClient users_api{control_plane_base_url};
  NewsApiClient news_api{control_plane_base_url};
  ProfileApiClient profile_api{control_plane_base_url};
  DocumentContentService document_service;

  // --- Network ---
  SignalingClient signaling_client;
  OAuthCallbackServer oauth_callback_server;

  // --- Lifetime ---
  std::shared_ptr<std::atomic_bool> alive_flag =
      std::make_shared<std::atomic_bool>(true);

  /// Rebuild API clients after env config is loaded.
  void RebuildApiClients() {
    room_api = RoomApiClient(control_plane_base_url);
    auth_api = AuthApiClient(control_plane_base_url);
    ice_client = IceConfigClient(control_plane_base_url);
    users_api = UsersApiClient(control_plane_base_url);
    news_api = NewsApiClient(control_plane_base_url);
    profile_api = ProfileApiClient(control_plane_base_url);
  }
};

}  // namespace tempolink::juceapp::di
