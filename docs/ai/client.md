# docs/ai/client.md

## 책임

Client는 사용자 입력, 오디오 장치, 실시간 오디오 처리, 네트워크 전송, 세션 UI를 담당한다.

## 원칙

- UI 상태와 실제 연결 상태를 분리한다.
- room joined, signaling connected, peer connected, audio streaming 상태를 구분한다.
- 실패를 UI 표시만으로 숨기지 않는다.
- 오디오 경로와 UI 로직을 섞지 않는다.

## 금지

- Audio callback 내부 blocking
- UI thread에서 장시간 network/audio 작업
- 연결 실패를 단순 success 상태로 처리
- 임의의 global session state 추가

## 중단 조건

- session state 구조 변경 필요
- audio transport 변경 필요
- protocol 변경 필요
- UI 변경이 실제 연결 상태 모델에 영향 있음

