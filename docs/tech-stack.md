## Client

| Category     | Stack           |
| ------------ | --------------- |
| Language     | C++20           |
| Framework    | JUCE            |
| Build System | CMake           |
| Networking   | standalone asio |
| Codec        | Opus            |
| Logging      | Quill           |
| Testing      | Catch2          |

### 선정이유

- Realtime-safe Audio Processing 구현 가능
- VST3 + Standalone App 동시 개발 가능
- Cross-platform Audio Layer 제공
- DAW/ASIO/CoreAudio 생태계와 높은 호환성
- 비동기 저지연 로깅 구조 사용 가능
- Native Audio 시스템과 궁합이 좋음

---

## Audio

| Category          | Stack              |
| ----------------- | ------------------ |
| Windows Audio API | ASIO               |
| macOS Audio API   | CoreAudio          |
| Plugin Format     | VST3               |
| Buffer Strategy   | Small Fixed Buffer |

### 선정이유

- OS Mixer 우회를 통한 저지연 처리
- Stable Audio Callback 제공
- DAW 직접 연동 가능
- 실시간 합주 환경에 적합

---

## Networking

| Category      | Stack           |
| ------------- | --------------- |
| Transport     | UDP             |
| Connection    | Direct P2P      |
| NAT Traversal | STUN + ICE      |
| Packet Format | Binary Protocol |

### 선정이유

- TCP 재전송 및 HOL Blocking 회피
- 최소 네트워크 홉 유지
- Audio Packet 전송 최적화
- Parsing/Allocation 비용 최소화

---

## Backend

| Category      | Stack       |
| ------------- | ----------- |
| Framework     | Spring Boot |
| Database      | PostgreSQL  |
| Cache         | Redis       |
| Communication | WebSocket   |

### 선정이유

- 인증/세션/룸 관리 확장 용이
- OAuth/JWT 생태계 활용 가능
- Presence/Friend 시스템 확장 가능
- Audio Critical Path와 완전히 분리된 영역

---

## Infrastructure

| Category   | Stack                |
| ---------- | -------------------- |
| Container  | Docker               |
| Monitoring | Prometheus + Grafana |
| Logging    | Loki                 |
| Region     | Seoul / Tokyo        |

### 선정이유

- 지역 기반 RTT 최소화 전략 대응
- 모니터링 및 장애 추적 구조 구성 가능
- 멀티 리전 확장 고려 가능
