# TempoLink

온라인 합주 플랫폼 모노레포.

## Repository Layout

```text
TempoLink/
  Cpp/                  # 실시간 오디오 데이터 플레인 (Client/P2P core)
  SpringBoot/           # 컨트롤 플레인 API (로그인/방관리 확장 영역)
  Docs/                 # 아키텍처/프로토콜/운영 문서
  Scripts/              # 빌드/실행/CI 보조 스크립트
  Assets/               # UI/오디오/테스트 리소스
```

## Current Components

1. `Cpp/Client/JuceApp`: JUCE 기반 클라이언트
2. `Cpp/Client/RealtimeCore`: C++ 실시간 세션 코어 + 콘솔 클라이언트
3. `Cpp/Shared`: 공통 C++ 코드
4. `SpringBoot/ControlPlaneService`: 컨트롤 플레인 API(현재는 방 생성/입장/퇴장 중심)

## C++ Build

```powershell
cmake --preset client-debug
cmake --build --preset client-debug
```

JUCE 클라이언트:

```powershell
cmake --preset juce-client-debug
cmake --build --preset juce-client-debug
```

Windows 초저지연 팁:
1. JUCE 앱에서 `Require ASIO (Windows)` 활성화
2. 버퍼 `64`(안정 시 `32`) + 48kHz 권장

## SpringBoot Build/Run

Wrapper 생성(1회):

```bash
./Scripts/build/spring-wrapper.sh
```

```bash
./Scripts/run/control-plane.sh
```

Windows PowerShell:

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\build\spring-wrapper.ps1
```

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\run\control-plane.ps1
```

## API (ControlPlaneService)

1. `POST /api/rooms`
2. `GET /api/rooms`
3. `GET /api/rooms/{roomCode}`
4. `POST /api/rooms/{roomCode}/join`
5. `POST /api/rooms/{roomCode}/leave`
