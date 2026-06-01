# AGENTS.md — TempoLink

## 프로젝트 기준

TempoLink는 저지연 온라인 합주 앱이다.

판단 우선순위:

1. 실시간 오디오 안정성
2. 낮은 지연시간
3. 명확한 연결 상태
4. 작은 변경 단위
5. 검증 가능한 구현
6. 단순한 사용자 흐름

## 아키텍처 경계

- Backend는 auth, room, participant state, signaling을 담당한다.
- Client는 audio device, realtime audio, transport, UI를 담당한다.
- WebSocket은 signaling/control 용도다.
- Audio transport는 P2P 우선이다.
- Relay/Fallback은 보조 경로다.
- VST는 독립 앱이 아니라 DAW audio bridge다.

## 작업 전 참조 문서 선택

Agent는 작업 전에 명령의 성격을 파악하고 관련 문서를 먼저 읽는다.

| 작업 내용                                           | 참조 문서                 |
| --------------------------------------------------- | ------------------------- |
| 계획 수립, 작업 분배, 막힘 판단                     | `docs/ai/supervisor.md`   |
| JUCE, UI, device, session client                    | `docs/ai/client.md`       |
| Spring Boot, auth, room, WebSocket                  | `docs/ai/backend.md`      |
| audio callback, latency, jitter, buffer             | `docs/ai/audio.md`        |
| message schema, signaling contract                  | `docs/ai/protocol.md`     |
| DAW, VST3, plugin bridge                            | `docs/ai/plugin.md`       |
| CMake, Gradle, CI, packaging                        | `docs/ai/build-ci.md`     |
| test, benchmark, metric                             | `docs/ai/verification.md` |
| code comment, decision note                         | `docs/ai/commenting.md`   |
| 미구현 기능, Mock, placeholder, contract-first 구현 | `docs/ai/mock.md`         |
| 작업 중 발생한 이슈, 진행 흐름, 결정, 삽질 기록     | `docs/ai/work-log.md`     |

여러 영역에 걸친 작업이면 관련 문서를 모두 확인한다.

예시:

- “방 입장 후 연결 상태 UI 표시”
  → `client.md`, `backend.md`, `protocol.md`

- “오디오 끊김 개선”
  → `audio.md`, `client.md`, `verification.md`

- “VST에서 DAW 입력 연결”
  → `plugin.md`, `audio.md`, `protocol.md`

## 금지 원칙

명시적 승인 없이 다음을 수행하지 않는다.

- Backend에 실시간 오디오 처리 추가
- P2P 우선 구조를 중앙 중계 구조로 변경
- Audio callback 내부 blocking / network I/O / file I/O 추가
- 무제한 queue 추가
- 인증 우회 추가
- secret, token, key 저장
- 테스트 삭제
- 측정 없는 성능 개선 주장
- 대규모 리팩터링

## 코드 주석 원칙

주석은 코드가 “무엇을 하는지”보다 “왜 이렇게 했는지”를 설명한다.

작성한다:

- 의사결정 이유
- 선택한 방식의 trade-off
- 임시 구현의 제거 조건
- 실시간 처리에서 위험을 피한 이유
- 성능/안정성 관련 제약

작성하지 않는다:

- 코드만 봐도 알 수 있는 설명
- 낡은 주석
- 근거 없는 성능 주장
- TODO만 있고 조건이 없는 메모

예시:

```cpp
// Use a bounded queue to prevent unbounded latency growth.
// When the network thread is slower than audio capture, old packets are dropped
// instead of blocking the audio callback.
```

## AI Agent 운영

AI 작업은 Supervisor-Worker 방식으로 수행한다.

- Supervisor: 계획, 설계 판단, 위험 판단, 검토
- Worker: 계획에 따른 구현, 테스트, 보고

Worker는 다음 상황에서 멈추고 Supervisor에게 질문한다.

- 지시 범위를 넘는 수정이 필요할 때
- protocol, auth, session, audio path에 영향이 있을 때
- 기존 구조를 바꿔야 할 때
- 테스트 실패 원인을 확신하지 못할 때
- 성능 수치 근거가 없을 때

## 완료 기준

작업 완료 시 다음을 보고한다.

- 작업 중 발생한 이슈와 흐름을 `docs/work-log/YYYY-MM-DD.md`에 기록했는지 확인한다.
- 변경 파일
- 변경 이유
- 변경된 동작
- 수행한 검증
- 남은 위험
