# docs/ai/protocol.md

## 책임

Client와 Backend 사이의 메시지 계약을 관리한다.

## 원칙

- 메시지 타입은 명확해야 한다.
- breaking change는 양쪽을 함께 수정한다.
- room state, signaling state, peer state를 혼동하지 않는다.
- 임시 문자열보다 명시적 enum/schema를 우선한다.

## 금지

- client/backend 중 한쪽만 protocol 수정
- 의미가 다른 상태를 하나의 필드로 합치기
- 실패 케이스 없는 success-only message 추가
- 버전 영향 검토 없는 message rename

## 중단 조건

- message type 추가/삭제/변경
- enum 변경
- payload 구조 변경
- 구버전 호환성 영향 있음
