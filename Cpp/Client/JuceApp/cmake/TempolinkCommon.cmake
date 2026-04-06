include_guard(GLOBAL)

function(tempolink_add_common_library)
  if(TARGET tempolink_common)
    return()
  endif()

  set(TEMPOLINK_JUCEAPP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Client/JuceApp")
  set(TEMPOLINK_COMMON_INCLUDE_DIR "${TEMPOLINK_JUCEAPP_ROOT}/include")
  set(TEMPOLINK_COMMON_SOURCE_DIR "${TEMPOLINK_JUCEAPP_ROOT}/src/common")

  add_library(tempolink_common STATIC)

  set(TEMPOLINK_COMMON_SOURCES
    ${TEMPOLINK_COMMON_SOURCE_DIR}/bridge/UdpBridgeProtocol.cpp
    ${TEMPOLINK_COMMON_SOURCE_DIR}/audio/AudioCodecFactory.cpp
    ${TEMPOLINK_COMMON_SOURCE_DIR}/audio/NullAudioCodec.cpp
    ${TEMPOLINK_COMMON_SOURCE_DIR}/audio/OpusCodec.cpp
    ${TEMPOLINK_COMMON_SOURCE_DIR}/net/PacketCodec.cpp
  )

  if(WIN32)
    list(APPEND TEMPOLINK_COMMON_SOURCES
      ${TEMPOLINK_COMMON_SOURCE_DIR}/net/UdpSocket_win.cpp
      ${TEMPOLINK_COMMON_SOURCE_DIR}/audio/factory/AudioInputFactoryWindows.cpp
      ${TEMPOLINK_COMMON_SOURCE_DIR}/audio/factory/AudioOutputFactoryWindows.cpp
      ${TEMPOLINK_COMMON_SOURCE_DIR}/audio/platform/windows/WASAPIDeviceEnumerator.cpp
      ${TEMPOLINK_COMMON_SOURCE_DIR}/audio/platform/windows/WASAPIAudioInputDevice.cpp
      ${TEMPOLINK_COMMON_SOURCE_DIR}/audio/platform/windows/WASAPIAudioOutputDevice.cpp
    )
  elseif(APPLE)
    list(APPEND TEMPOLINK_COMMON_SOURCES
      ${TEMPOLINK_COMMON_SOURCE_DIR}/net/UdpSocket_posix.cpp
      ${TEMPOLINK_COMMON_SOURCE_DIR}/audio/factory/AudioInputFactoryMacOS.cpp
      ${TEMPOLINK_COMMON_SOURCE_DIR}/audio/factory/AudioOutputFactoryMacOS.cpp
      ${TEMPOLINK_COMMON_SOURCE_DIR}/audio/platform/macos/CoreAudioDeviceEnumerator.cpp
      ${TEMPOLINK_COMMON_SOURCE_DIR}/audio/platform/macos/CoreAudioInputDevice.cpp
      ${TEMPOLINK_COMMON_SOURCE_DIR}/audio/platform/macos/CoreAudioOutputDevice.cpp
    )
  elseif(UNIX)
    list(APPEND TEMPOLINK_COMMON_SOURCES
      ${TEMPOLINK_COMMON_SOURCE_DIR}/net/UdpSocket_posix.cpp
      ${TEMPOLINK_COMMON_SOURCE_DIR}/audio/factory/AudioInputFactoryLinux.cpp
      ${TEMPOLINK_COMMON_SOURCE_DIR}/audio/factory/AudioOutputFactoryLinux.cpp
      ${TEMPOLINK_COMMON_SOURCE_DIR}/audio/platform/linux/ALSAAudioInputDevice.cpp
      ${TEMPOLINK_COMMON_SOURCE_DIR}/audio/platform/linux/ALSAAudioOutputDevice.cpp
    )
  else()
    list(APPEND TEMPOLINK_COMMON_SOURCES
      ${TEMPOLINK_COMMON_SOURCE_DIR}/net/UdpSocket_posix.cpp
      ${TEMPOLINK_COMMON_SOURCE_DIR}/audio/factory/AudioInputFactoryGeneric.cpp
      ${TEMPOLINK_COMMON_SOURCE_DIR}/audio/factory/AudioOutputFactoryGeneric.cpp
      ${TEMPOLINK_COMMON_SOURCE_DIR}/audio/platform/generic/NullAudioInputDevice.cpp
      ${TEMPOLINK_COMMON_SOURCE_DIR}/audio/platform/generic/NullAudioOutputDevice.cpp
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
      ${TEMPOLINK_COMMON_INCLUDE_DIR}
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
endfunction()

