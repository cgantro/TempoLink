# docs/ai/mock.md

## 목적

Mock은 미구현 기능을 실제 구현처럼 속이기 위한 코드가 아니다.

Mock은 다음 목적에만 사용한다.

- protocol/interface 합의
- UI 또는 session 흐름 검증
- 실제 구현 전 연결 지점 확보
- 병렬 개발을 위한 임시 데이터 제공

## 기본 원칙

구현되지 않은 기능은 추측으로 완성하지 않는다.

필요한 경우 다음 순서로 처리한다.

1. protocol, interface, state를 먼저 정의한다.
2. 실제 구현 위치에는 뼈대만 남긴다.
3. 흐름 검증이 필요하면 Mock 데이터를 사용한다.
4. Mock 사용 이유와 제거 조건을 주석으로 남긴다.
5. 실제 구현 시 연결될 대상 interface를 명시한다.

## 허용되는 Mock

- Backend 미구현 상태에서 client 흐름 검증
- P2P transport 미구현 상태에서 session state 검증
- VST 연동 전 DAW input flow 검증
- DB 연동 전 response contract 검증
- UI 개발을 위한 임시 room/participant 데이터

## 금지되는 Mock

- 실제 연결 성공처럼 보이는 fake success
- 인증 우회 Mock
- 실패 케이스 없는 happy path 전용 Mock
- 제거 조건 없는 Mock
- 운영 코드와 구분되지 않는 Mock
- 성능 측정에 사용하는 Mock
- architecture 경계를 흐리는 Mock

## Mock 작성 규칙

Mock을 추가할 때는 다음 정보를 남긴다.

```txt
Mock 목적:
대체하는 실제 기능:
실제 구현 연결 지점:
제거 조건:
```

## 코드 주석 예시

```cpp
// Mock:
// Purpose: Validate session UI flow before P2P transport is implemented.
// Replaces: Real peer connection state from transport layer.
// Connects to: IPeerConnectionStateProvider.
// Remove when: Transport emits PeerConnected / PeerDisconnected events.
```

```java
// Mock:
// Purpose: Provide room response contract before persistence is implemented.
// Replaces: RoomRepository-backed lookup.
// Connects to: RoomRepository.
// Remove when: RoomRepository integration is complete.
```

## 판단 기준

Mock은 흐름을 열어두기 위한 임시 장치다.

Mock 때문에 다음이 흐려지면 안 된다.

- 실제 연결 상태
- protocol 계약
- auth/session 정책
- audio transport 책임
- backend/client 경계

## Worker 중단 조건

Worker는 다음 상황에서 임의로 Mock을 추가하지 않는다.

- 인증 관련 Mock이 필요할 때
- 실제 연결 성공처럼 보이는 상태가 필요할 때
- protocol 변경이 필요한 때
- audio path에 Mock이 들어가는 때
- 성능 측정에 Mock 데이터가 필요한 때
- 제거 조건을 정의할 수 없을 때

이 경우 Supervisor에게 먼저 질문한다.

## 완료 보고

Mock을 추가한 작업은 완료 보고에 다음을 포함한다.

```txt
추가한 Mock:
Mock 목적:
대체한 실제 기능:
실제 구현 연결 지점:
제거 조건:
남은 위험:
```
