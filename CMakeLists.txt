cmake_minimum_required(VERSION 3.24)

project(TempoLink LANGUAGES C CXX)

# Smart Build Acceleration: Use sccache if available on the system
find_program(SCCACHE_PROGRAM sccache)
if(SCCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${SCCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER "${SCCACHE_PROGRAM}")
    message(STATUS "Build acceleration enabled: Using sccache")
endif()

add_subdirectory(Cpp)
