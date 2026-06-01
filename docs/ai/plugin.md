# docs/ai/plugin.md

## 책임

Plugin은 DAW audio를 TempoLink로 연결하는 bridge다.

## 원칙

- Plugin은 독립 앱이 아니다.
- 가능한 shared/core logic을 재사용한다.
- DAW audio thread 제약을 우선한다.
- Plugin UI는 최소 설정 중심으로 둔다.

## 금지

- Plugin 내부에 독립 room/session system 구현
- Desktop client 기능 전체 복제
- Audio thread에서 blocking/network/file I/O
- DAW host 안정성을 해치는 작업

## 중단 조건

- plugin이 직접 networking을 가져야 할 때
- shared audio core 변경 필요
- DAW thread와 TempoLink thread 경계 변경
- VST 역할 확대 필요
