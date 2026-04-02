cmake_minimum_required(VERSION 3.24)

project(TempoLink LANGUAGES C CXX)

# Smart Build Acceleration: Use sccache if available on the system
find_program(SCCACHE_PROGRAM sccache)
if(SCCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${SCCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER "${SCCACHE_PROGRAM}")
    
    # MSVC와 sccache 충돌 방지: 디버그 정보(/Zi) 대신 오브젝트 임베드(/Z7) 포맷 사용
    if(MSVC)
        cmake_policy(SET CMP0141 NEW)
        set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>")
    endif()
    
    message(STATUS "Build acceleration enabled: Using sccache")
endif()

add_subdirectory(Cpp)
