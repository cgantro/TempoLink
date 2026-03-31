# control-plane-service (Spring Boot, no-auth MVP)

Auth 없이 빠르게 방 관리만 제공하는 컨트롤 플레인 서비스입니다.

## Endpoints

1. `POST /api/rooms` - 방 생성
2. `GET /api/rooms` - 방 목록
3. `GET /api/rooms/{roomCode}` - 방 상세
4. `POST /api/rooms/{roomCode}/join` - 방 입장
5. `POST /api/rooms/{roomCode}/leave` - 방 퇴장
6. `GET /api/system/relay-endpoint` - 오디오 릴레이 주소 조회
7. `GET /actuator/health` - 헬스체크

## Example Requests

```http
POST /api/rooms
Content-Type: application/json

{
  "hostUserId": "user-a",
  "maxParticipants": 4,
  "relayHost": "127.0.0.1",
  "relayPort": 40000
}
```

```http
POST /api/rooms/ABC123/join
Content-Type: application/json

{
  "userId": "user-b"
}
```

## Run

Gradle Wrapper가 있으면(권장):

```bash
./gradlew bootRun
```

Gradle CLI만 있는 경우:

```bash
gradle bootRun
```

또는 IDE(IntelliJ)에서 `ControlPlaneApplication` 실행.
