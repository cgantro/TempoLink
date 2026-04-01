# control-plane-service (Spring Boot, signaling-first MVP)

방 관리/시그널링 + 소셜 로그인(OAuth: Google) 기반 컨트롤 플레인 서비스입니다.

## Service Boundary

- 서버 책임: 방 생성/입장/퇴장, 시그널링 메시지 중계
- 클라이언트 책임: P2P 연결 수립 이후의 실제 커스텀 전송 로직
- 현재 범위: WebRTC 미디어 처리 서버 미포함 (시그널링 전달만 수행)
- 개발 DB: H2 in-memory 사용 (서버 재시작 시 데이터 초기화)
- 인증: OAuth2 로그인 (Google) + one-time login ticket 교환

## Endpoints

1. `POST /api/rooms` - 방 생성
2. `GET /api/rooms` - 방 목록
3. `GET /api/rooms/{roomCode}` - 방 상세
4. `POST /api/rooms/{roomCode}/join` - 방 입장
5. `POST /api/rooms/{roomCode}/leave` - 방 퇴장
6. `WS /ws/signaling?roomCode={code}&userId={id}` - 시그널링 채널
7. `GET /api/system/relay-endpoint` - 오디오 릴레이 주소 조회
8. `GET /actuator/health` - 헬스체크
9. `GET /api/auth/providers` - 사용 가능한 소셜 로그인 공급자 조회
10. `GET /api/auth/social/{provider}/start` - 소셜 로그인 시작 (브라우저 리다이렉트)
11. `GET /api/auth/social/ticket/{ticket}` - one-time 티켓 교환

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

```json
// WS /ws/signaling message example
{
  "type": "transport.offer",
  "toUserId": "user-b",
  "payload": {
    "custom": "p2p-bootstrap-data"
  }
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

## Environment File (.env.server)

Spring Boot가 `application.yml`에서 아래 설정으로 `.env.server`를 자동 import 합니다.

```yaml
spring:
  config:
    import:
      - optional:file:.env.server[.properties]
      - optional:file:../.env.server[.properties]
      - optional:file:../../.env.server[.properties]
```

즉, 서버 설정은 `.env.server`(properties 형식) -> `application.yml` placeholder -> `@ConfigurationProperties` 로 바인딩됩니다.

## OAuth (Google) 설정

필수 환경 변수 (`.env.server`):

```bash
GOOGLE_CLIENT_ID=...
GOOGLE_CLIENT_SECRET=...
```

참고: 아래 Spring 표준 키도 동일하게 지원됩니다.

```bash
SPRING_SECURITY_OAUTH2_CLIENT_REGISTRATION_GOOGLE_CLIENT_ID=...
SPRING_SECURITY_OAUTH2_CLIENT_REGISTRATION_GOOGLE_CLIENT_SECRET=...
```

선택 환경 변수:

```bash
TEMPOLINK_AUTH_SUCCESS_REDIRECT=tempolink://auth/callback
TEMPOLINK_AUTH_TICKET_TTL=300
TEMPOLINK_AUTH_SESSION_TTL=86400
```

로그인 흐름:

1. 클라이언트가 `/api/auth/providers`로 사용 가능 공급자 확인
2. 브라우저에서 `/api/auth/social/google/start?redirectUri=tempolink://auth/callback` 호출
3. 로그인 성공 시 `redirectUri?ticket=...&provider=...` 로 리다이렉트
4. 클라이언트가 `/api/auth/social/ticket/{ticket}` 호출로 세션 토큰/프로필 교환

## Dev DB (H2)

- JDBC URL: `jdbc:h2:mem:tempolink;MODE=PostgreSQL;DB_CLOSE_DELAY=-1;DB_CLOSE_ON_EXIT=FALSE`
- Console: `http://localhost:8080/h2-console`
- Username: `sa` / Password: (empty)

포트 충돌 시 예: `SERVER_PORT=8081 ./gradlew bootRun`
