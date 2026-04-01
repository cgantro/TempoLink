#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

#include "tempolink/client/ClientSession.h"
#include "tempolink/config/NetworkConstants.h"

int main() {
  tempolink::client::ClientSession::Config config;
  config.server_host = tempolink::config::kDefaultRelayHost;
  config.server_port = tempolink::config::kDefaultRelayPort;
  config.room_id = tempolink::config::kDefaultRoomId;
  config.participant_id = 9001;
  config.nickname = "console-client";

  tempolink::client::ClientSession session;
  if (!session.Start(config)) {
    std::cerr << "[client-console] failed to start session\n";
    return 1;
  }
  if (!session.Join()) {
    std::cerr << "[client-console] failed to join room\n";
    return 1;
  }

  std::cout << "[client-console] joined room=" << config.room_id
            << " user=" << config.participant_id
            << " backend=" << session.AudioBackendName() << "\n";

  for (;;) {
    session.Tick();
    const auto stats = session.GetStats();
    if (stats.connected) {
      std::cout << "[client-console] rtt=" << stats.last_rtt_ms << " ms\n";
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}
