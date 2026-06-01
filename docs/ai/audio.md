# docs/ai/audio.md

## 책임

실시간 오디오 경로의 안정성과 지연시간을 유지한다.

## 원칙

- Audio callback은 빠르게 반환한다.
- Audio thread와 network thread를 분리한다.
- Queue는 bounded로 둔다.
- Overflow policy를 명시한다.
- 성능 개선은 측정값으로만 주장한다.

## 금지

- Audio callback 내부 blocking
- Audio callback 내부 network I/O
- Audio callback 내부 file I/O
- 반복적 heap allocation
- 무제한 queue
- 측정 없는 latency 개선 주장

## 중단 조건

- buffer size 변경
- jitter buffer 정책 변경
- queue overflow 정책 변경
- encoder/decoder 경로 변경
- packet scheduling 변경
