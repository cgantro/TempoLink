# Local P2P Connection Test (Two Clients, One Machine or LAN)

## 목적
- 클라이언트 간 P2P signaling 경로가 정상인지 확인한다.
- 세션 화면의 연결 배지(`P2P / Reconnecting / Failed`)와 peer latency(ms) 갱신을 검증한다.

## 사전 조건
- Spring Boot backend 실행 중
- 두 클라이언트가 같은 backend에 로그인 가능
- 같은 Room에 입장 가능

## 테스트 절차
1. 클라이언트 A 실행 후 로그인한다.
2. 클라이언트 B 실행 후 로그인한다.  
   - 같은 계정 대신 서로 다른 사용자 ID가 보이도록 로그인해야 peer가 분리된다.
3. A/B를 같은 방에 입장시킨다.
4. 세션 화면에서 참가자 행(`ParticipantRow`)을 확인한다.
5. 2~3초 내에 상대방 `latencyMs`가 `-1`에서 실제 수치로 바뀌는지 확인한다.
6. 상대방 행의 `Reconnect` 버튼을 눌러 ping 재시도 후 latency 갱신 여부를 확인한다.

## 기대 결과
- peer latency가 갱신되면 해당 참가자 연결 배지가 `P2P`로 표시된다.
- peer가 아직 응답하지 않으면 `Reconnecting`으로 유지된다.
- signaling 자체가 끊기면 `Failed`로 표시된다.
- 참가자가 1명(나만 입장)인 상태에서는 `Unknown`/대기 상태로 표시되고
  불필요한 `Reconnecting` 루프가 발생하지 않는다.

## 실패 시 체크
- backend signaling endpoint(`/ws/signaling`) 연결 상태
- 방 참가자 목록에 실제로 두 사용자가 존재하는지
- 양쪽 클라이언트 시간이 크게 어긋나지 않았는지
- 방화벽이 localhost/LAN websocket 통신을 차단하는지
