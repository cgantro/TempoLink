# docs/ai/commenting.md

## 목적

주석은 코드의 의사결정을 보존하기 위해 작성한다.

코드가 “무엇을 하는지”는 코드로 표현한다.
주석은 “왜 이렇게 했는지”를 설명한다.

## 작성해야 하는 주석

- 다른 선택지 대신 현재 방식을 고른 이유
- 성능, 안정성, 실시간성 때문에 선택한 trade-off
- 임시 구현과 제거 조건
- 위험한 코드의 안전 조건
- protocol 호환성 때문에 유지하는 처리
- fallback 조건

## 피해야 하는 주석

- 코드 반복 설명
- 오래된 주석
- 근거 없는 성능 설명
- “나중에 수정”만 적힌 TODO
- 실제 동작과 다른 설명

## 주석 형식

```txt
// Decision:
// Reason:
// Trade-off:
// Remove when:
```

## 예시

```cpp
// Decision: Drop oldest packets when the send queue is full.
// Reason: Blocking here can increase audio latency and destabilize playback.
// Trade-off: Some packets may be lost, but latency remains bounded.
```

```java
// Decision: Keep roomJoined and signalingConnected as separate states.
// Reason: A user can join a room while WebSocket signaling is disconnected.
// Trade-off: UI state becomes more explicit but requires additional transitions.
```
