# docs/ai/backend.md

## 책임

Backend는 Control Plane이다.

담당 범위:

- auth
- room create/join/leave
- participant state
- signaling
- future persistence

## 원칙

- room state와 signaling state를 구분한다.
- WebSocket handler와 domain logic을 분리한다.
- session lifecycle을 명확히 한다.
- client가 복구 가능한 에러를 반환한다.

## 금지

- 실시간 오디오 처리 추가
- REST API를 audio packet 경로로 사용
- 인증 우회
- room state를 임의 in-memory 전역 상태로 확장
- 실패 원인을 숨기는 broad catch

## 중단 조건

- auth/session 정책 변경 필요
- DB schema 변경 필요
- signaling message 변경 필요
- room lifecycle 변경 필요
