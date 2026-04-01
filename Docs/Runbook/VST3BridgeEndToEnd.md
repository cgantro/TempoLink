# VST3 Bridge End-to-End Test Flow

## 목적
- DAW에서 VST3 `TempoLink VST3 Bridge` 플러그인으로 입력된 신호가
  JUCE 클라이언트 `AudioBridgePort`로 전달되고,
  다시 클라이언트 믹스 출력이 DAW 트랙 출력으로 돌아오는지 검증한다.

## 브리지 구성
- `TEMPOLINK_BRIDGE_HOST`: 브리지 상대 호스트 (기본 `127.0.0.1`)
- `TEMPOLINK_BRIDGE_CLIENT_LISTEN_PORT`: 클라이언트 수신 포트 (기본 `49010`)
- `TEMPOLINK_BRIDGE_PLUGIN_LISTEN_PORT`: 플러그인 수신 포트 (기본 `49011`)

클라이언트와 플러그인은 같은 값으로 맞춰야 한다.
VST3 플러그인은 DAW 프로세스 환경변수를 읽기 때문에, 필요 시 OS 환경변수로 동일 값을 설정한다.

## 빌드 타겟
- JUCE 앱: `tempolink_juce_client`
- VST3 플러그인: `tempolink_vst3_bridge_VST3`

`TEMPOLINK_BUILD_JUCE_VST3_BRIDGE=ON`일 때 플러그인 타겟이 생성된다.

## 테스트 절차
1. Spring Boot 백엔드 실행 후 로그인/방 API가 정상인지 확인한다.
2. JUCE 클라이언트를 실행하고 방 입장까지 완료한다.
3. DAW에서 오디오 트랙(또는 버스)에 `TempoLink VST3 Bridge`를 인서트한다.
4. DAW 트랙에 신호(마이크/악기/테스트 톤)를 넣는다.
5. 클라이언트 세션에서 내 입력 레벨이 DAW 신호에 반응하는지 확인한다.
6. 같은 방의 원격 오디오(또는 테스트 신호)가 DAW 트랙 출력으로 되돌아오는지 확인한다.

## 성공 기준
- DAW 입력을 보냈을 때 클라이언트 입력 레벨/송신 경로가 반응한다.
- 클라이언트 믹스 출력이 플러그인 출력으로 돌아와 DAW 미터에 표시된다.
- 브리지 미연결 시 플러그인 출력은 무음(또는 클리어) 상태를 유지한다.

## 트러블슈팅
- 포트 충돌: 다른 프로세스가 `49010/49011`을 점유하면 수신 실패.
- 샘플레이트 불일치: DAW와 클라이언트 오디오 포맷이 크게 다르면 프레임 드롭 가능.
- 방 미입장 상태: 클라이언트 세션이 비활성이면 플러그인 반환 오디오가 없을 수 있음.
