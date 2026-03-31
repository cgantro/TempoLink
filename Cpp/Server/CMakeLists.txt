add_executable(tempolink_server)

target_sources(
  tempolink_server
  PRIVATE
    src/main.cpp
    src/server/ClockSyncService.cpp
    src/server/RoomRegistry.cpp
    src/server/UdpRelayServer.cpp
)

target_include_directories(
  tempolink_server
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(tempolink_server PRIVATE tempolink_common)
target_compile_features(tempolink_server PRIVATE cxx_std_20)
