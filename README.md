# TempoLink

> Ultra Low-Latency P2P Realtime Ensemble Platform

TempoLink는 실시간 온라인 합주를 위한 초저지연 P2P 오디오 플랫폼입니다.

기존 화상회의 플랫폼은 대화 중심 구조이기 때문에,
합주에 필요한 수준의 지연시간을 만족시키기 어렵습니다.

TempoLink는:

- Native 기반 오디오 처리
- UDP 기반 P2P 통신
- 최소한의 네트워크 홉
- 오디오 세션과 GUI 분리

구조를 통해 실시간 합주 환경을 목표로 합니다.

벤치마킹:

- Yamaha Syncroom

목표 레이턴시:

- 한국: 15ms 이하
- 일본: 30ms 이하

---

# Features

- Ultra Low-Latency Audio Streaming
- Direct P2P Audio Connection
- Native Windows/macOS Client
- GUI / Audio Session Process Separation
- UDP 기반 실시간 오디오 전송
- Opus 기반 저지연 오디오 압축

---

# Philosophy

TempoLink는 “항상 연결된다”보다:

> “연결되었을 때 충분히 빠르다”

를 목표로 합니다.

이를 위해:

- Relay 서버를 사용하지 않으며
- Direct P2P 연결만 지원합니다.

따라서 일부 NAT 환경에서는 연결이 실패할 수 있습니다.

하지만:

- 추가 네트워크 홉 제거
- RTT 최소화
- 실시간성 유지

를 우선하는 방향을 선택했습니다.

---

# Architecture

```text
GUI Process (Qt)
        │
       IPC
        │
Audio Session Process (JUCE + UDP + Opus)
        │
 Direct P2P UDP
        │
      Peer
```

---

# Platform Support

- Windows
- macOS

---

# Status

In Progress

---

# Documentation

- `/docs/architecture.md`
- `/docs/networking.md`
- `/docs/audio-pipeline.md`
- `/docs/tech-stack.md`

---

# License

MIT
