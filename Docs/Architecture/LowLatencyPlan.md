# Low Latency Plan (Target: < 10ms)

## Latency Budget (same region)

1. Input device: 1~3ms (ASIO/CoreAudio/ALSA low buffer)
2. Network: 2~5ms (same region, UDP direct path)
3. Output device: 1~3ms (ASIO/CoreAudio/ALSA low buffer)

합계 목표: 10ms 이하.

## What is implemented now

1. JUCE 클라이언트:
   - Windows에서 `Require ASIO` 옵션 제공
   - 버퍼 크기(32/64/128/256)와 샘플레이트(44.1k/48k/96k) 적용
   - 실제 활성 드라이버 타입/버퍼/샘플레이트 상태 표시
2. 전송:
   - UDP 사용
   - 현재는 릴레이 서버 기반(Client-Server) 경로

## Current hard limits

1. `Cpp/Shared`의 Windows 오디오 백엔드는 아직 mock 구현(`WASAPI-MOCK`) 상태다.
2. 중앙 릴레이만 사용하면 P2P 대비 추가 RTT가 발생한다.

## Next required milestones

1. Windows:
   - 실 구현 ASIO 입력/출력 어댑터(또는 JUCE 경로로 완전 통합)
2. Linux/macOS:
   - ALSA/CoreAudio 실제 low-latency 경로 검증
3. 네트워크:
   - ICE/STUN 기반 P2P 모드 추가(실패 시 릴레이 fallback)
4. DSP/Codec:
   - 2.5~5ms 프레임 단위 코덱 파이프라인 검증

## Operational recommendations

1. 같은 리전 배포 + 유선 LAN 사용
2. 48kHz / 64 samples부터 시작해 하드웨어 안정성에 따라 32로 조정
3. Wi-Fi 환경은 기본적으로 10ms 목표 달성이 어려움
