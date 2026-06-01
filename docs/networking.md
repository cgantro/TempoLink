# /docs/networking.md

# Networking

TempoLink는 초저지연 실시간 합주를 위해 UDP 기반 P2P 구조를 사용합니다.

---

# Why UDP?

TCP는:

- 재전송 발생
- Head-of-Line Blocking
- 지연 누적

문제가 존재합니다.

실시간 합주에서는 늦게 도착한 패킷보다:

- 일부 손실되더라도
- 빠르게 전달되는 것

이 더 중요합니다.

따라서 TempoLink는 UDP 기반으로 설계되었습니다.

---

# P2P Connection

TempoLink는 Direct P2P 연결만 지원합니다.

```text id="pn93xh"
Peer A  <------UDP------>  Peer B
```

오디오 데이터는 중앙 서버를 거치지 않습니다.

장점:

- RTT 감소
- 추가 홉 제거
- Relay Queue Delay 제거

---

# NAT Traversal

P2P 연결을 위해:

- STUN
- ICE

를 사용합니다.

TURN Relay는 사용하지 않습니다.

따라서:

- Symmetric NAT
- 일부 Carrier NAT
- 방화벽 환경

에서는 연결 실패 가능성이 존재합니다.

---

# Why No Relay?

TURN Relay는 연결 성공률은 높일 수 있습니다.

하지만:

- RTT 증가
- 추가 네트워크 홉
- Relay Queue Delay

문제가 발생합니다.

TempoLink는:

- “항상 연결되는 것”
  보다
- “충분히 빠른 연결”

을 우선합니다.

---

# Target Latency

| Region | Target RTT |
| ------ | ---------- |
| Korea  | < 15ms     |
| Japan  | < 30ms     |

---

# Packet Flow

```text id="f1rjwa"
Audio Capture
    ↓
Opus Encode
    ↓
UDP Packetization
    ↓
P2P Transmission
    ↓
Jitter Buffer
    ↓
Opus Decode
    ↓
Audio Playback
```

---

# Future Considerations

- Adaptive Jitter Buffer
- Packet Loss Concealment
- Forward Error Correction
- Congestion Control
- Network Quality Estimation
