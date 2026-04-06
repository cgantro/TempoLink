cmake_minimum_required(VERSION 3.24)

option(TEMPOLINK_FETCH_JUCE "Download JUCE automatically when not found" ON)
set(TEMPOLINK_JUCE_VERSION "8.0.7" CACHE STRING "JUCE version used when TEMPOLINK_FETCH_JUCE=ON")
set(TEMPOLINK_JUCE_SOURCE_DIR "" CACHE PATH "Local JUCE source directory (contains CMakeLists.txt)")
option(TEMPOLINK_BUILD_JUCE_VST3_BRIDGE "Build TempoLink JUCE VST3 bridge plugin" ON)

include(cmake/TempolinkJuceDeps.cmake)
include(cmake/TempolinkTargetSettings.cmake)
include(cmake/TempolinkJuceClientSources.cmake)
include(cmake/TempolinkVstBridgeSources.cmake)
include(cmake/TempolinkInstall.cmake)
include(cmake/TempolinkPackaging.cmake)

tempolink_ensure_juce_available()
find_package(OpenSSL REQUIRED)

juce_add_gui_app(tempolink_juce_client PRODUCT_NAME "TempoLink JUCE Client")

target_sources(
  tempolink_juce_client
  PRIVATE
    ${TEMPOLINK_JUCE_CLIENT_SOURCES}
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
    OpenSSL::SSL
    OpenSSL::Crypto
)

tempolink_apply_common_target_settings(tempolink_juce_client "${CMAKE_CURRENT_SOURCE_DIR}")

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
      ${TEMPOLINK_VST_BRIDGE_SOURCES}
  )

  target_link_libraries(
    tempolink_vst3_bridge
    PRIVATE
      juce::juce_audio_processors
      juce::juce_audio_utils
      juce::juce_gui_extra
      tempolink_common
  )

  tempolink_apply_common_target_settings(tempolink_vst3_bridge "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

tempolink_configure_install(tempolink_juce_client)
tempolink_configure_packaging()
