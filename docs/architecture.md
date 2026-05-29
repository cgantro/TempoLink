# /docs/architecture.md

# Architecture

TempoLink는 초저지연 실시간 합주를 목표로 하는 P2P 기반 Native 오디오 플랫폼입니다.

핵심 목표:

- 최소 RTT 확보
- 오디오 지연 최소화
- Audio Thread 안정성 확보
- Direct P2P Audio Path 유지

---

# High Level Architecture

```text id="9y4m1t"
+--------------------------------------------------+
|                TempoLink Client                  |
|--------------------------------------------------|
| GUI Thread                                       |
| - Room UI                                        |
| - Device Settings                                |
| - Network Status                                 |
|--------------------------------------------------|
| Network Thread                                   |
| - UDP Packet TX/RX                               |
| - P2P Session Handling                           |
|--------------------------------------------------|
| Audio Thread                                     |
| - Audio Capture                                  |
| - Opus Encode/Decode                             |
| - Jitter Buffer                                  |
| - Audio Playback                                 |
+--------------------------------------------------+
                    |
             Direct P2P UDP
                    |
+--------------------------------------------------+
|                Remote Peer                       |
+--------------------------------------------------+
```

---

# Thread Separation

TempoLink는:

- GUI
- Network
- Audio

를 서로 다른 스레드로 분리합니다.

목적:

- Audio Callback 보호
- GUI 부하 격리
- Network Blocking 방지
- Realtime-safe 환경 유지

실시간 오디오 시스템에서는:

- UI Freeze
- Rendering Spike
- Blocking IO

등도 오디오 품질에 영향을 줄 수 있습니다.

따라서 Audio Thread는 최대한 독립적으로 동작하도록 설계합니다.

---

# Audio Thread

Audio Thread는 가장 높은 우선순위를 가집니다.

역할:

- Audio Capture
- Buffer Processing
- Opus Encoding
- Playback

Realtime Audio Thread에서는:

- Lock
- Heap Allocation
- Blocking Operation

을 최소화해야 합니다.

---

# Network Thread

역할:

- UDP Packet 송수신
- Peer 상태 관리
- Session 상태 처리

특징:

- Audio Thread와 분리
- Packet Queue 기반 전달

---

# GUI Thread

역할:

- 사용자 인터페이스
- Room 상태 표시
- Device Settings
- Network 상태 표시

특징:

- 오디오 처리 직접 수행하지 않음

---

# Server Structure

TempoLink는 중앙 Media Relay 서버를 사용하지 않습니다.

서버 역할:

- Signaling
- Peer 정보 교환
- Session Discovery

실제 오디오 데이터는 Peer 간 직접 전송됩니다.

---

# Design Philosophy

TempoLink는:

- 연결 성공률
  보다
- 실시간성
  을 우선합니다.

따라서:

- TURN Relay 미사용
- Direct P2P 우선
- 실패 가능한 구조 허용

전략을 채택합니다.
