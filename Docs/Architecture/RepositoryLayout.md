# TempoLink Repository Layout

## Goal

온라인 합주 플랫폼을 데이터 플레인(C++)과 컨트롤 플레인(Spring Boot)으로 분리해,
실시간 성능 요구사항과 운영/확장 요구사항을 동시에 만족시키는 모노레포 구조를 사용한다.

## Final Structure

```text
TempoLink/
  Cpp/
    CMakeLists.txt
    cmake/
    Shared/
      include/
      src/
    Client/
      RealtimeCore/
        include/
        src/
      JuceApp/
        src/
  SpringBoot/
    ControlPlaneService/
      src/main/java
      src/main/resources
      build.gradle.kts
      settings.gradle.kts
  Docs/
    Architecture/
    Api/
    Protocol/
    Runbook/
    ADR/
  Scripts/
    build/
    run/
    ci/
  Assets/
    AudioSamples/
    Ui/
    Logos/
    TestData/
```

## Separation Rules

1. `Cpp/Shared`:
   - 네트워크 패킷/UDP, 오디오 코덱 추상화, 플랫폼 오디오 인터페이스 같은 공통 코드만 둔다.
2. `Cpp/Client`:
   - `RealtimeCore`: 실시간 세션 코어 + 디버깅용 콘솔 클라이언트
   - `JuceApp`: 실제 데스크탑 앱(UI + 오디오 + UDP)
3. `SpringBoot/ControlPlaneService`:
   - 로그인/권한 이전 MVP 단계의 방 생성/입장/퇴장 API

## Build Management

1. 루트 `CMakeLists.txt`는 C++ 서브트리 진입점(`add_subdirectory(Cpp)`)만 담당한다.
2. `Cpp/CMakeLists.txt`에서 `Client/RealtimeCore`, `Client/JuceApp` 빌드를 옵션으로 분리한다.
3. Spring Boot는 `SpringBoot/ControlPlaneService` 내부에서 독립적으로 `Gradle`로 빌드한다.
4. 루트 `CMakePresets.json`으로 Windows/Linux/macOS 프리셋을 공통 관리한다.

## Safe Move Procedure (for future refactors)

1. 새 폴더를 먼저 만들고(`Cpp`, `SpringBoot`, `Docs`, `Scripts`, `Assets`) 빈 상태로 커밋한다.
2. 공통 코드(`Shared`)를 먼저 이동하고 include 경로를 고정한다.
3. 클라이언트 코드를 이동하고 Console 앱 단독 빌드로 검증한다.
4. JUCE 앱 경로를 이동하고 별도 프리셋으로 검증한다.
5. Spring Boot 코드를 이동하고 API 실행/헬스체크를 확인한다.
6. 마지막에 README, 스크립트, CI 경로를 업데이트한다.

## Notes

1. 플랫폼별 오디오 백엔드는 소스 파일 분리(`windows/macos/linux`) 원칙을 유지하고,
   런타임 코드에서는 인터페이스 팩토리(`CreateDefaultAudioInputDevice`, `CreateDefaultAudioOutputDevice`)만 사용한다.
2. 대규모 ifdef를 비즈니스 로직에 섞지 않고 빌드/팩토리 레이어에서 캡슐화한다.
