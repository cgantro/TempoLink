cmake_minimum_required(VERSION 3.24)

option(TEMPOLINK_FETCH_JUCE "Download JUCE automatically when not found" ON)
set(TEMPOLINK_JUCE_VERSION "8.0.7" CACHE STRING "JUCE version used when TEMPOLINK_FETCH_JUCE=ON")
set(TEMPOLINK_JUCE_SOURCE_DIR "" CACHE PATH "Local JUCE source directory (contains CMakeLists.txt)")

find_package(JUCE CONFIG QUIET)

if(NOT TARGET juce::juce_core)
  if(TEMPOLINK_JUCE_SOURCE_DIR AND EXISTS "${TEMPOLINK_JUCE_SOURCE_DIR}/CMakeLists.txt")
    add_subdirectory("${TEMPOLINK_JUCE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/_deps/juce-src")
  elseif(TEMPOLINK_FETCH_JUCE)
    include(FetchContent)
    set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
    set(JUCE_ENABLE_MODULE_SOURCE_GROUPS ON CACHE BOOL "" FORCE)
    FetchContent_Declare(
      juce
      URL https://github.com/juce-framework/JUCE/archive/refs/tags/${TEMPOLINK_JUCE_VERSION}.tar.gz
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(juce)
  else()
    message(FATAL_ERROR
      "JUCE was not found.\n"
      "Set JUCE_DIR/CMAKE_PREFIX_PATH, or set TEMPOLINK_JUCE_SOURCE_DIR, or enable TEMPOLINK_FETCH_JUCE."
    )
  endif()
endif()

juce_add_gui_app(tempolink_juce_client PRODUCT_NAME "TempoLink JUCE Client")

target_sources(
  tempolink_juce_client
  PRIVATE
    include/tempolink/juce/MainComponent.h
    include/tempolink/juce/app/ClientAppPresenter.h
    include/tempolink/juce/app/ParticipantRosterBuilder.h
    include/tempolink/juce/network/IceConfigClient.h
    include/tempolink/juce/network/RoomApiClient.h
    include/tempolink/juce/network/SignalingClient.h
    include/tempolink/juce/style/UiStyle.h
    include/tempolink/juce/ui/LobbyView.h
    include/tempolink/juce/ui/ParticipantStripComponent.h
    include/tempolink/juce/ui/RoomEntrySettingsView.h
    include/tempolink/juce/ui/RoomCardComponent.h
    include/tempolink/juce/ui/SessionView.h
    include/tempolink/juce/ui/UiModels.h
    include/tempolink/juce/ui/common/ConnectionBadge.h
    include/tempolink/juce/ui/common/MetricLabel.h
    include/tempolink/juce/ui/common/PanelCard.h
    include/tempolink/juce/ui/session/MainOutPanel.h
    include/tempolink/juce/ui/session/MyInputPanel.h
    include/tempolink/juce/ui/session/TopStatusBar.h
    src/AppMain.cpp
    src/MainComponent.cpp
    src/app/ClientAppPresenter.cpp
    src/app/ParticipantRosterBuilder.cpp
    src/network/IceConfigClient.cpp
    src/network/RoomApiClient.cpp
    src/network/SignalingClient.cpp
    src/ui/LobbyView.cpp
    src/ui/ParticipantStripComponent.cpp
    src/ui/RoomEntrySettingsView.cpp
    src/ui/RoomCardComponent.cpp
    src/ui/SessionView.cpp
    src/ui/common/ConnectionBadge.cpp
    src/ui/common/MetricLabel.cpp
    src/ui/common/PanelCard.cpp
    src/ui/session/MainOutPanel.cpp
    src/ui/session/MyInputPanel.cpp
    src/ui/session/TopStatusBar.cpp
)

target_compile_features(tempolink_juce_client PRIVATE cxx_std_20)
target_include_directories(
  tempolink_juce_client
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
  tempolink_juce_client
  PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_devices
    juce::juce_gui_extra
    tempolink_client_runtime
)

target_compile_definitions(
  tempolink_juce_client
  PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)
