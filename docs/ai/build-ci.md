# docs/ai/build-ci.md

## 책임

빌드, 테스트, 패키징, CI 안정성을 관리한다.

## 원칙

- 로컬 빌드와 CI 빌드 차이를 줄인다.
- dependency 변경은 이유를 남긴다.
- packaging 변경은 산출물 영향을 확인한다.
- CI 최적화는 검증 누락으로 이어지면 안 된다.

## 금지

- 빌드를 통과시키기 위한 테스트 삭제
- 실패 로그 숨기기
- secret을 workflow나 repo에 저장
- 검증 없는 dependency 교체
- 플랫폼별 빌드 차이 무시

## 중단 조건

- CMake toolchain 변경
- Gradle 설정 변경
- vcpkg/dependency 구조 변경
- release packaging 변경
- GitHub Actions required check 변경
