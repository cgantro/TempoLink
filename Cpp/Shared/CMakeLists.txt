add_library(tempolink_common STATIC)

set(TEMPOLINK_COMMON_SOURCES
  src/bridge/UdpBridgeProtocol.cpp
  src/audio/AudioCodecFactory.cpp
  src/audio/NullAudioCodec.cpp
  src/audio/OpusCodec.cpp
  src/net/PacketCodec.cpp
)

if(WIN32)
  list(APPEND TEMPOLINK_COMMON_SOURCES
    src/net/UdpSocket_win.cpp
    src/audio/factory/AudioInputFactoryWindows.cpp
    src/audio/factory/AudioOutputFactoryWindows.cpp
    src/audio/platform/windows/WASAPIDeviceEnumerator.cpp
    src/audio/platform/windows/WASAPIAudioInputDevice.cpp
    src/audio/platform/windows/WASAPIAudioOutputDevice.cpp
  )
elseif(APPLE)
  list(APPEND TEMPOLINK_COMMON_SOURCES
    src/net/UdpSocket_posix.cpp
    src/audio/factory/AudioInputFactoryMacOS.cpp
    src/audio/factory/AudioOutputFactoryMacOS.cpp
    src/audio/platform/macos/CoreAudioDeviceEnumerator.cpp
    src/audio/platform/macos/CoreAudioInputDevice.cpp
    src/audio/platform/macos/CoreAudioOutputDevice.cpp
  )
elseif(UNIX)
  list(APPEND TEMPOLINK_COMMON_SOURCES
    src/net/UdpSocket_posix.cpp
    src/audio/factory/AudioInputFactoryLinux.cpp
    src/audio/factory/AudioOutputFactoryLinux.cpp
    src/audio/platform/linux/ALSAAudioInputDevice.cpp
    src/audio/platform/linux/ALSAAudioOutputDevice.cpp
  )
else()
  list(APPEND TEMPOLINK_COMMON_SOURCES
    src/net/UdpSocket_posix.cpp
    src/audio/factory/AudioInputFactoryGeneric.cpp
    src/audio/factory/AudioOutputFactoryGeneric.cpp
    src/audio/platform/generic/NullAudioInputDevice.cpp
    src/audio/platform/generic/NullAudioOutputDevice.cpp
  )
endif()

target_sources(
  tempolink_common
  PRIVATE
    ${TEMPOLINK_COMMON_SOURCES}
)

target_include_directories(
  tempolink_common
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(tempolink_common PUBLIC cxx_std_20)

if(TEMPOLINK_ENABLE_BOOST)
  find_package(Boost 1.74 REQUIRED COMPONENTS system)
  target_link_libraries(tempolink_common PUBLIC Boost::system)
  target_compile_definitions(tempolink_common PUBLIC TEMPOLINK_HAS_BOOST=1)
endif()

if(TEMPOLINK_ENABLE_OPUS)
  find_package(Opus QUIET)

  if(Opus_FOUND)
    target_link_libraries(tempolink_common PUBLIC Opus::opus)
    target_compile_definitions(tempolink_common PUBLIC TEMPOLINK_HAS_OPUS=1)
  else()
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
      pkg_check_modules(OPUS opus IMPORTED_TARGET)
    endif()

    if(OPUS_FOUND)
      target_link_libraries(tempolink_common PUBLIC PkgConfig::OPUS)
      target_compile_definitions(tempolink_common PUBLIC TEMPOLINK_HAS_OPUS=1)
    else()
      message(WARNING "TEMPOLINK_ENABLE_OPUS=ON but Opus library not found.")
    endif()
  endif()
endif()

if(WIN32)
  target_link_libraries(tempolink_common PUBLIC ws2_32 ole32 uuid avrt)
elseif(APPLE)
  target_link_libraries(tempolink_common PUBLIC
    "-framework CoreAudio"
    "-framework CoreFoundation"
  )
endif()
