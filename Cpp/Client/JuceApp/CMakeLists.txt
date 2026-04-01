cmake_minimum_required(VERSION 3.24)

option(TEMPOLINK_FETCH_JUCE "Download JUCE automatically when not found" ON)
set(TEMPOLINK_JUCE_VERSION "8.0.7" CACHE STRING "JUCE version used when TEMPOLINK_FETCH_JUCE=ON")
set(TEMPOLINK_JUCE_SOURCE_DIR "" CACHE PATH "Local JUCE source directory (contains CMakeLists.txt)")
option(TEMPOLINK_FETCH_CPPHTTPLIB "Download cpp-httplib automatically when not found" ON)
set(TEMPOLINK_CPPHTTPLIB_VERSION "v0.18.6" CACHE STRING "cpp-httplib version")
option(TEMPOLINK_FETCH_NLOHMANN_JSON "Download nlohmann_json automatically when not found" ON)
set(TEMPOLINK_NLOHMANN_JSON_VERSION "v3.11.3" CACHE STRING "nlohmann_json version")
option(TEMPOLINK_BUILD_JUCE_VST3_BRIDGE "Build TempoLink JUCE VST3 bridge plugin" ON)

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

find_package(httplib CONFIG QUIET)

if(NOT TARGET httplib::httplib)
  if(TEMPOLINK_FETCH_CPPHTTPLIB)
    include(FetchContent)
    FetchContent_Declare(
      cpp_httplib
      URL https://github.com/yhirose/cpp-httplib/archive/refs/tags/${TEMPOLINK_CPPHTTPLIB_VERSION}.tar.gz
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(cpp_httplib)
  else()
    message(FATAL_ERROR
      "cpp-httplib was not found.\n"
      "Set httplib_DIR/CMAKE_PREFIX_PATH, or enable TEMPOLINK_FETCH_CPPHTTPLIB."
    )
  endif()
endif()

find_package(nlohmann_json CONFIG QUIET)

if(NOT TARGET nlohmann_json::nlohmann_json)
  if(TEMPOLINK_FETCH_NLOHMANN_JSON)
    include(FetchContent)
    FetchContent_Declare(
      nlohmann_json
      URL https://github.com/nlohmann/json/archive/refs/tags/${TEMPOLINK_NLOHMANN_JSON_VERSION}.tar.gz
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(nlohmann_json)
  else()
    message(FATAL_ERROR
      "nlohmann_json was not found.\n"
      "Set nlohmann_json_DIR/CMAKE_PREFIX_PATH, or enable TEMPOLINK_FETCH_NLOHMANN_JSON."
    )
  endif()
endif()

juce_add_gui_app(tempolink_juce_client PRODUCT_NAME "TempoLink JUCE Client")

target_sources(
  tempolink_juce_client
  PRIVATE
    include/tempolink/juce/MainComponent.h
    include/tempolink/juce/app/ClientAppPresenter.h
    include/tempolink/juce/app/LobbyBindings.h
    include/tempolink/juce/app/ParticipantRosterBuilder.h
    include/tempolink/juce/app/RoomCatalog.h
    include/tempolink/juce/app/RoomCommandController.h
    include/tempolink/juce/app/SessionBindings.h
    include/tempolink/juce/app/SessionModelSupport.h
    include/tempolink/juce/bridge/UdpAudioBridgePort.h
    include/tempolink/juce/config/ClientEnvConfig.h
    include/tempolink/juce/constants/ApiPaths.h
    include/tempolink/juce/constants/AuthConstants.h
    include/tempolink/juce/constants/AudioSettingsConstants.h
    include/tempolink/juce/constants/ClientText.h
    include/tempolink/juce/logging/AppLogger.h
    include/tempolink/juce/model/RoomSummary.h
    include/tempolink/juce/network/auth/AuthApiClient.h
    include/tempolink/juce/network/http/common/DocumentContentService.h
    include/tempolink/juce/network/ice/IceConfigClient.h
    include/tempolink/juce/network/ice/IceConfigJsonMapper.h
    include/tempolink/juce/network/ice/IceConfigTypes.h
    include/tempolink/juce/network/http/common/HttpTransport.h
    include/tempolink/juce/network/http/common/JsonParse.h
    include/tempolink/juce/network/http/news/NewsApiClient.h
    include/tempolink/juce/network/auth/OAuthCallbackServer.h
    include/tempolink/juce/network/http/room/RoomApiClient.h
    include/tempolink/juce/network/http/room/RoomApiJsonMapper.h
    include/tempolink/juce/network/http/room/RoomApiPayloadBuilder.h
    include/tempolink/juce/network/http/room/RoomApiTypes.h
    include/tempolink/juce/network/signaling/SignalingClient.h
    include/tempolink/juce/network/signaling/SignalingEventParser.h
    include/tempolink/juce/network/http/user/UsersApiClient.h
    include/tempolink/juce/network/http/user/ProfileApiClient.h
    include/tempolink/juce/style/UiStyle.h
    include/tempolink/juce/ui/screens/LobbyView.h
    include/tempolink/juce/ui/screens/LoginView.h
    include/tempolink/juce/ui/screens/ManualView.h
    include/tempolink/juce/ui/screens/MyRoomsView.h
    include/tempolink/juce/ui/screens/NewsView.h
    include/tempolink/juce/ui/components/ParticipantStripComponent.h
    include/tempolink/juce/ui/screens/ProfileView.h
    include/tempolink/juce/ui/screens/QnaView.h
    include/tempolink/juce/ui/screens/RoomEntrySettingsView.h
    include/tempolink/juce/ui/components/RoomCardComponent.h
    include/tempolink/juce/ui/screens/SessionView.h
    include/tempolink/juce/ui/screens/SettingsView.h
    include/tempolink/juce/ui/models/UiModels.h
    include/tempolink/juce/ui/screens/UsersView.h
    include/tempolink/juce/ui/common/ConnectionBadge.h
    include/tempolink/juce/ui/common/MetricLabel.h
    include/tempolink/juce/ui/common/PanelCard.h
    include/tempolink/juce/ui/session/MainOutPanel.h
    include/tempolink/juce/ui/session/MyInputPanel.h
    include/tempolink/juce/ui/session/TopStatusBar.h
    src/AppMain.cpp
    src/MainComponent.cpp
    src/app/ClientAppPresenter.cpp
    src/app/LobbyBindings.cpp
    src/app/ParticipantRosterBuilder.cpp
    src/app/RoomCatalog.cpp
    src/app/RoomCommandController.cpp
    src/app/SessionBindings.cpp
    src/app/SessionModelSupport.cpp
    src/config/ClientEnvConfig.cpp
    src/bridge/UdpAudioBridgePort.cpp
    src/logging/AppLogger.cpp
    src/network/auth/AuthApiClient.cpp
    src/network/http/common/DocumentContentService.cpp
    src/network/ice/IceConfigClient.cpp
    src/network/ice/IceConfigJsonMapper.cpp
    src/network/http/common/HttpTransport.cpp
    src/network/http/common/JsonParse.cpp
    src/network/http/news/NewsApiClient.cpp
    src/network/auth/OAuthCallbackServer.cpp
    src/network/http/room/RoomApiClient.cpp
    src/network/http/room/RoomApiJsonMapper.cpp
    src/network/http/room/RoomApiPayloadBuilder.cpp
    src/network/signaling/SignalingClient.cpp
    src/network/signaling/SignalingEventParser.cpp
    src/network/http/user/UsersApiClient.cpp
    src/network/http/user/ProfileApiClient.cpp
    src/ui/screens/LobbyView.cpp
    src/ui/screens/LoginView.cpp
    src/ui/screens/ManualView.cpp
    src/ui/screens/MyRoomsView.cpp
    src/ui/screens/NewsView.cpp
    src/ui/components/ParticipantStripComponent.cpp
    src/ui/screens/ProfileView.cpp
    src/ui/screens/QnaView.cpp
    src/ui/screens/RoomEntrySettingsView.cpp
    src/ui/components/RoomCardComponent.cpp
    src/ui/screens/SessionView.cpp
    src/ui/screens/SettingsView.cpp
    src/ui/screens/UsersView.cpp
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
    ${CMAKE_SOURCE_DIR}/Cpp/Shared/include
)

target_link_libraries(
  tempolink_juce_client
  PRIVATE
    httplib::httplib
    nlohmann_json::nlohmann_json
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

if(TEMPOLINK_BUILD_JUCE_VST3_BRIDGE)
  juce_add_plugin(
    tempolink_vst3_bridge
    COMPANY_NAME "TempoLink"
    PRODUCT_NAME "TempoLink VST3 Bridge"
    BUNDLE_ID "com.tempolink.vst3bridge"
    PLUGIN_MANUFACTURER_CODE Tplk
    PLUGIN_CODE Tvb1
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Fx
    FORMATS VST3
  )

  target_sources(
    tempolink_vst3_bridge
    PRIVATE
      include/tempolink/juce/vstbridge/VstBridgeAudioProcessor.h
      src/vstbridge/VstBridgeAudioProcessor.cpp
  )

  target_include_directories(
    tempolink_vst3_bridge
    PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/include
      ${CMAKE_SOURCE_DIR}/Cpp/Shared/include
  )

  target_compile_features(tempolink_vst3_bridge PRIVATE cxx_std_20)

  target_link_libraries(
    tempolink_vst3_bridge
    PRIVATE
      juce::juce_audio_processors
      juce::juce_audio_utils
      juce::juce_gui_extra
      tempolink_common
  )

  target_compile_definitions(
    tempolink_vst3_bridge
    PRIVATE
      JUCE_WEB_BROWSER=0
      JUCE_USE_CURL=0
  )
endif()
