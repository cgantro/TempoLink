# TempoLink

> Native low-latency ensemble client for Windows and macOS

TempoLink는 실시간 온라인 합주를 위한 초저지연 P2P 오디오 클라이언트다.

기존 화상회의 플랫폼은 대화 중심 구조이기 때문에,
합주에 필요한 수준의 지연시간과 오디오 제어를 만족시키기 어렵다.

TempoLink는 다음 방향을 기준으로 설계한다.

- `Qt Quick/QML` 기반 네이티브 데스크톱 UI
- `JUCE/C++` 기반 오디오 엔진 유지
- `UDP` 기반 direct P2P 연결
- UI 스레드와 오디오/네트워크 스레드 분리

---

# Why This Stack

- 화면은 `Qt/QML`로 옮겨 디자인 밀도, 텍스트 대비, 상태 바인딩을 더 직접적으로 제어한다.
- 오디오 세션은 `JUCE/C++`로 유지해 ASIO/CoreAudio 대응과 실시간 callback 구조를 보존한다.
- 네트워크는 `standalone Asio + UDP`로 유지해 직접 연결과 낮은 전송 지연을 우선한다.

상세 내용은 [docs/tech-stack.md](docs/tech-stack.md)에서 관리한다.

---

# Features

- Native Windows/macOS desktop client
- Qt Quick/QML based session UI
- JUCE/C++ based audio device and session engine
- Direct P2P UDP audio path
- Low-latency device monitoring and session diagnostics

---

# Target Latency

- Korea: `15ms` 이하
- Japan: `30ms` 이하

---

# Architecture

```text
+--------------------------------------+
|          TempoLink Desktop App       |
|--------------------------------------|
| UI Thread        (Qt Quick / QML)    |
| Session State    (QObject facade)    |
| Network Thread   (Asio UDP)          |
| Audio Thread     (JUCE / C++)        |
+--------------------------------------+
                   |
            Direct P2P UDP
                   |
                 Peer
```

---

# Platform Support

- Windows
- macOS

---

# Status

In Progress

- UI stack migration: `JUCE UI -> Qt/QML`
- Audio session engine: `C++/JUCE 유지`
- Networking: `Asio UDP 유지`

---

# Documentation

- [docs/architecture.md](docs/architecture.md)
- [docs/networking.md](docs/networking.md)
- [docs/audio-pipeline.md](docs/audio-pipeline.md)
- [docs/tech-stack.md](docs/tech-stack.md)

---

# License

MIT
