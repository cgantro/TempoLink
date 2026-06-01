include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

add_executable(TempoLinkTests
    Audio/JitterBufferTest.cpp
)

target_include_directories(TempoLinkTests PRIVATE ../include/tempolink)
target_link_libraries(TempoLinkTests PRIVATE Catch2::Catch2WithMain tempolink_core)

include(CTest)
include(Catch)
catch_discover_tests(TempoLinkTests)
