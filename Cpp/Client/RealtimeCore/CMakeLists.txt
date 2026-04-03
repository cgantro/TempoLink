add_library(tempolink_client_runtime STATIC)

target_sources(
  tempolink_client_runtime
  PRIVATE
    src/client/AudioPipeline.cpp
    src/client/ClientSession.cpp
    src/client/ClientTransport.cpp
    src/client/ClockSyncTracker.cpp
    src/client/EndpointManager.cpp
    src/client/PeerJitterBuffer.cpp
    src/client/audio/MetronomeProcessor.cpp
    src/client/audio/ReverbProcessor.cpp
    src/client/audio/MultiStreamMixer.cpp
    src/client/audio/InputGainProcessor.cpp
    src/client/audio/LevelMeter.cpp
    src/client/audio/AudioProcessorChain.cpp
    src/client/audio/AudioDeviceManager.cpp
)

target_include_directories(
  tempolink_client_runtime
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(tempolink_client_runtime PUBLIC tempolink_common)
target_compile_features(tempolink_client_runtime PUBLIC cxx_std_20)

if(TEMPOLINK_BUILD_CLIENT)
  add_executable(tempolink_client_console)
  target_sources(
    tempolink_client_console
    PRIVATE
      src/main_console.cpp
  )
  target_link_libraries(tempolink_client_console PRIVATE tempolink_client_runtime)
  target_compile_features(tempolink_client_console PRIVATE cxx_std_20)
endif()
