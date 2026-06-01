# /docs/audio-pipeline.md

# Audio Pipeline

TempoLink는 실시간 합주를 위해 저지연 오디오 파이프라인을 구성합니다.

---

# Audio Flow

```text id="gx3mpe"
Audio Input
    ↓
Audio Buffer
    ↓
Opus Encoder
    ↓
UDP Transmission
    ↓
Jitter Buffer
    ↓
Opus Decoder
    ↓
Audio Output
```

---

# Audio Input

오디오 입력은:

- ASIO (Windows)
- CoreAudio (macOS)

를 사용합니다.

목표:

- 낮은 Buffer Size 유지
- Stable Audio Callback 확보

---

# Audio Buffer

버퍼가 커질수록:

- 안정성은 증가하지만
- 레이턴시는 증가합니다.

TempoLink는:

- 가능한 작은 Buffer 유지
- 안정성과 레이턴시 균형 확보

를 목표로 합니다.

---

# Opus Encoding

TempoLink는 Opus Codec을 사용합니다.

이유:

- Low Latency
- 높은 압축 효율
- Packet Loss 내성

실시간 음성 및 악기 처리에 적합합니다.

---

# Jitter Buffer

네트워크 환경에서는 패킷 도착 시간이 일정하지 않습니다.

Jitter Buffer는:

- 패킷 도착 시간 보정
- Playback 안정화

를 담당합니다.

하지만 버퍼가 커질수록 레이턴시가 증가합니다.

따라서 최소 수준으로 유지하는 것이 중요합니다.

---

# Audio Thread Safety

실시간 오디오 스레드에서는:

- Lock
- Heap Allocation
- Blocking IO

를 최소화해야 합니다.

오디오 처리 중:

- GC Pause
- Memory Spike
- Thread Blocking

이 발생하면:

- Click Noise
- Audio Drop
- Sync Drift

문제가 발생할 수 있습니다.

---

# Design Goals

- Stable Audio Callback
- Low Latency
- Minimal Jitter
- Realtime-safe Processing
- Low Memory Overhead
