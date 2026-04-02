#define TEMPOLINK_USE_OPUS // 빌드 시스템에 선언되어 있지 않을 경우를 대비해 Opus 활성화

#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>
#include <vector>
#include <stdexcept>

#include "tempolink/client/ClientSession.h"
#include "tempolink/config/NetworkConstants.h"
#include "tempolink/client/codec/OpusCodec.h"

int main() {
  tempolink::client::ClientSession::Config config;
  config.server_host = tempolink::config::kDefaultRelayHost;
  config.server_port = tempolink::config::kDefaultRelayPort;
  config.room_id = tempolink::config::kDefaultRoomId;
  config.participant_id = 9001;
  config.nickname = "console-client";
  
  // --- JUCE UI에서 설정된 오디오 옵션 연동 ---
  config.sample_rate_hz = 48000; // 44.1kHz, 48kHz 등 UI에서 선택된 값
  config.channels = 2;
  config.frame_size = 480;       // Buffer size (e.g., 10ms at 48kHz)

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

  // --- Opus 코덱 연동 및 작동 테스트 ---
  try {
    tempolink::client::codec::OpusAudioEncoder encoder(config.sample_rate_hz, config.channels);
    tempolink::client::codec::OpusAudioDecoder decoder(config.sample_rate_hz, config.channels);
    encoder.setBitrate(64000); // 64kbps 설정

    // 10ms 분량의 더미 오디오 데이터 (0.1f) 생성
    std::vector<float> dummy_pcm(config.frame_size * config.channels, 0.1f);
    std::vector<uint8_t> encoded_packet;
    std::vector<float> decoded_pcm;

    int encoded_bytes = encoder.encode(dummy_pcm.data(), config.frame_size, encoded_packet);
    if (encoded_bytes > 0) {
        int decoded_samples = decoder.decode(encoded_packet.data(), encoded_bytes, decoded_pcm, config.frame_size);
        std::cout << "[client-console] Opus codec linked successfully: Encoded " << encoded_bytes 
                  << " bytes, Decoded " << decoded_samples << " samples.\n";
    }
  } catch (const std::exception& e) {
    std::cerr << "[client-console] Opus codec link error: " << e.what() << "\n";
  }

  for (;;) {
    session.Tick();
    const auto stats = session.GetStats();
    if (stats.connected) {
      std::cout << "[client-console] rtt=" << stats.last_rtt_ms << " ms\n";
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}
