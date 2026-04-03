cmake_minimum_required(VERSION 3.24)

# MSVC와 sccache 충돌 방지: 디버그 정보(/Zi) 대신 오브젝트 임베드(/Z7) 포맷 사용
# 반드시 project() 선언 전에 설정해야 기본 플래그(/Zi)가 안전하게 덮어씌워집니다.
cmake_policy(SET CMP0141 NEW)
if(POLICY CMP0207)
    cmake_policy(SET CMP0207 NEW)
endif()
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>")

project(TempoLink LANGUAGES C CXX)

# Smart Build Acceleration: Use sccache if available on the system
find_program(SCCACHE_PROGRAM sccache)
if(SCCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${SCCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER "${SCCACHE_PROGRAM}")
    
    message(STATUS "Build acceleration enabled: Using sccache")
endif()

# --- 의존성 관리 (FetchContent & System Packages) ---
# Windows 환경에서 기본 OpenSSL 설치 경로(Chocolatey 등)를 자동으로 찾도록 설정
if (WIN32 AND NOT DEFINED OPENSSL_ROOT_DIR AND NOT DEFINED ENV{OPENSSL_ROOT_DIR})
    if (EXISTS "C:/vcpkg/installed/x64-windows")
        set(OPENSSL_ROOT_DIR "C:/vcpkg/installed/x64-windows")
    elseif (EXISTS "C:/vcpkg/installed/x64-windows-static")
        set(OPENSSL_ROOT_DIR "C:/vcpkg/installed/x64-windows-static")
    elseif (EXISTS "C:/Program Files/OpenSSL-Win64")
        set(OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL-Win64")
    elseif (EXISTS "C:/Program Files/OpenSSL-Win32")
        set(OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL-Win32")
    endif()
endif()

find_package(OpenSSL REQUIRED)

include(FetchContent)

# nlohmann-json (헤더 온리)
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(nlohmann_json)

# cpp-httplib (헤더 온리, OpenSSL 활성화)
set(HTTPLIB_REQUIRE_OPENSSL ON CACHE BOOL "" FORCE)
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.15.3
)
FetchContent_MakeAvailable(httplib)

# --- 선택적 코덱 및 유틸리티 라이브러리 (정적 링크) ---
option(TEMPOLINK_ENABLE_OPUS "Enable Opus codec" OFF)
if(TEMPOLINK_ENABLE_OPUS)
    message(STATUS "Fetching Opus Codec (Static)...")
    # DLL이 생성되지 않도록 무조건 정적 라이브러리(Static)로 빌드를 강제합니다.
    set(OPUS_BUILD_SHARED_LIBRARY OFF CACHE BOOL "" FORCE)
    set(OPUS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(OPUS_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
    
    FetchContent_Declare(
        opus
        GIT_REPOSITORY https://github.com/xiph/opus.git
        GIT_TAG v1.4
    )
    FetchContent_MakeAvailable(opus)
    
    # Shared 모듈의 OpusCodec이 활성화되도록 전역 매크로 정의
    add_compile_definitions(TEMPOLINK_HAS_OPUS)
endif()

option(TEMPOLINK_ENABLE_BOOST "Enable Boost" OFF)
if(TEMPOLINK_ENABLE_BOOST)
    message(STATUS "Fetching Boost (Static)...")
    # 최신 Boost는 CMake를 지원하므로 FetchContent로 쉽게 정적 링크가 가능합니다.
    FetchContent_Declare(
        boost
        URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz
    )
    FetchContent_MakeAvailable(boost)
endif()

add_subdirectory(Cpp)
