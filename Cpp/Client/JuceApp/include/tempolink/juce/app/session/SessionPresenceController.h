#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "tempolink/client/ClientSession.h"
#include "tempolink/juce/network/ice/IceConfigTypes.h"
#include "tempolink/juce/network/signaling/SignalingClient.h"
#include "tempolink/juce/ui/models/UiModels.h"
#include "tempolink/juce/ui/interfaces/ISessionView.h"

class SessionPresenceController {
 public:
  SessionPresenceController(ISessionView& session_view,
                            tempolink::client::ClientSession& session,
                            SignalingClient& signaling_client);

  void reset();
  void initializeParticipants(const std::vector<std::string>& user_ids,
                              const std::string& current_user_id,
                              const std::string& selected_part_label,
                              bool self_muted);
  void updateSelfMuted(bool muted);
  void updateParticipantMonitorVolume(const std::string& user_id, float volume);
  void updateParticipantMonitorPan(const std::string& user_id, float pan);
  void refreshAudioDevices(bool session_active);
  void refreshViewState(bool session_active, const std::string& active_room_code,
                        int tick_index, bool signaling_connected,
                        bool ice_config_loaded,
                        const IceConfigSnapshot& ice_config);
  void sendPeerPings(bool session_active);
  bool sendReconnectProbe(const std::string& user_id, bool session_active);
  void handleSignalingEvent(bool session_active,
                            const std::string& active_room_code,
                            const std::string& current_user_id,
                            const std::string& selected_part_label,
                            const SignalingClient::Event& event);
  const std::vector<ParticipantSummary>& participants() const;

 private:
  void rebuildParticipantsFromUserIds(const std::vector<std::string>& user_ids,
                                      const std::string& current_user_id,
                                      const std::string& selected_part_label,
                                      bool self_muted);
  static std::uint64_t nowSteadyMs();

  ISessionView& session_view_;
  tempolink::client::ClientSession& session_;
  SignalingClient& signaling_client_;

  std::vector<ParticipantSummary> participants_;
  std::unordered_map<std::string, int> peer_latency_ms_;
  std::unordered_map<std::string, std::uint64_t> peer_ping_last_sent_ms_;
  bool has_last_signaling_connected_ = false;
  bool last_signaling_connected_ = false;
  bool has_last_media_connected_ = false;
  bool last_media_connected_ = false;
  int last_peer_rtt_count_ = -1;
  std::chrono::steady_clock::time_point last_peer_ping_tick_ =
      std::chrono::steady_clock::time_point::min();
  std::chrono::steady_clock::time_point last_device_refresh_tick_ =
      std::chrono::steady_clock::time_point::min();
};
