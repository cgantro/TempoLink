## Client

| Category     | Stack                |
| ------------ | -------------------- |
| Language     | C++20                |
| UI           | Qt 6 + Qt Quick/QML  |
| Audio Engine | JUCE                 |
| Build System | CMake                |
| Networking   | standalone Asio      |
| Testing      | Catch2               |

### 선정이유

- `Qt Quick/QML`은 macOS/Windows 공통 데스크톱 UI를 빠르게 구성하면서도, 디자인 토큰과 커스텀 컴포넌트로 화면 밀도와 대비를 세밀하게 맞추기 쉽다.
- `QML <-> C++` 연결이 직접적이어서, 세션 상태와 오디오 장치 상태를 `QObject` 파사드로 노출하는 구조를 만들기 좋다.
- `JUCE`는 UI가 아니라 오디오 디바이스 접근과 callback 기반 오디오 엔진에 집중시켜, ASIO/CoreAudio 대응과 실시간 처리 안정성을 유지한다.
- `standalone Asio`는 UDP 기반 P2P 세션과 저수준 비동기 네트워크 제어에 적합하다.
- `CMake`는 Qt와 JUCE를 함께 묶는 단일 빌드 그래프를 만들기 쉽고, Windows/macOS 프리셋 관리가 단순하다.
- `Catch2`는 코어 라이브러리 단위 테스트를 가볍게 붙이기에 적합하다.

---

## UI

| Category        | Stack                      |
| --------------- | -------------------------- |
| App Shell       | Qt Quick Controls          |
| Scene Rendering | QML + custom components    |
| State Binding   | QObject / QAbstractListModel |
| Design Tokens   | QML singleton theme        |

### 선정이유

- 웹뷰 없이 네이티브 데스크톱 UI를 유지할 수 있다.
- 시작 화면, 로비, 룸, 오디오, 네트워크 화면을 공통 컴포넌트로 나누기 좋다.
- 텍스트 대비, 카드 밀도, 세그먼트 탭, 메트릭 표시 같은 합주 앱 특화 UI를 직접 제어하기 쉽다.
- QML 애니메이션과 상태 바인딩으로 실시간 상태 변화를 자연스럽게 보여줄 수 있다.

---

## Audio

| Category          | Stack              |
| ----------------- | ------------------ |
| Windows Audio API | ASIO               |
| macOS Audio API   | CoreAudio          |
| Engine Layer      | JUCE AudioDeviceManager |
| Buffer Strategy   | Small Fixed Buffer |

### 선정이유

- OS 오디오 계층과 직접 가깝게 붙어 낮은 지연시간을 확보하기 쉽다.
- 실시간 callback 모델을 유지하면서 UI 프레임워크와 오디오 엔진을 분리할 수 있다.
- Windows와 macOS에서 동일한 C++ 오디오 코드를 유지하기 쉽다.
- 이후 Opus 인코딩/디코딩과 지터 버퍼를 같은 코어 라이브러리에 붙이기 좋다.

---

## Networking

| Category      | Stack           |
| ------------- | --------------- |
| Transport     | UDP             |
| Connection    | Direct P2P      |
| NAT Traversal | STUN + ICE      |
| Packet Format | Binary Protocol |

### 선정이유

- TCP 재전송과 HOL blocking을 피하고, 실시간 오디오에 유리한 지연 특성을 확보할 수 있다.
- 중앙 미디어 릴레이 없이 직접 경로를 유지하기 좋다.
- 오디오 패킷과 세션 상태를 분리해 제어하기 쉽다.
- low-level socket 제어와 스레드 분리에 유리하다.

---

## Backend

| Category      | Stack       |
| ------------- | ----------- |
| Role          | Signaling / Session Discovery |
| Communication | WebSocket   |
| Auth          | OAuth / JWT candidate |
| Storage       | PostgreSQL / Redis candidate |

### 선정이유

- 오디오 패스와 분리된 control plane으로 유지하기 쉽다.
- 룸 탐색, 인증, peer 정보 교환 같은 세션 관리 기능을 확장하기 좋다.
- 실시간 오디오 전송 실패와 계정/세션 기능을 분리해 운영할 수 있다.

---

## Infrastructure

| Category   | Stack                |
| ---------- | -------------------- |
| Container  | Docker               |
| Monitoring | Prometheus + Grafana |
| Logging    | Loki                 |
| Region     | Seoul / Tokyo        |

### 선정이유

- 지역 기반 RTT 최적화 전략과 잘 맞는다.
- 연결 품질, 지연 시간, 세션 상태를 수집하고 추적하기 좋다.
- 멀티 리전 확장과 장애 분석 구조를 미리 고려할 수 있다.
