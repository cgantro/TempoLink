cmake_minimum_required(VERSION 3.24)

project(TempoLinkCpp VERSION 0.1.0 LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(TEMPOLINK_BUILD_SERVER "Build TempoLink UDP relay server" ON)
option(TEMPOLINK_BUILD_CLIENT "Build TempoLink C++ realtime client core" ON)
option(TEMPOLINK_BUILD_JUCE_CLIENT "Build JUCE-based client app" OFF)
option(TEMPOLINK_ENABLE_BOOST "Link Boost.System for network extensions" OFF)
option(TEMPOLINK_ENABLE_OPUS "Enable Opus codec integration" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC AND CMAKE_GENERATOR MATCHES "Ninja")
  # Avoid C1041 PDB write contention on parallel MSVC+Ninja builds.
  add_compile_options(
    $<$<CONFIG:Debug>:/Z7>
    $<$<CONFIG:RelWithDebInfo>:/Z7>
  )
endif()

add_subdirectory(Shared)

if(TEMPOLINK_BUILD_SERVER)
  add_subdirectory(Server)
endif()

if(TEMPOLINK_BUILD_CLIENT OR TEMPOLINK_BUILD_JUCE_CLIENT)
  add_subdirectory(Client/RealtimeCore)
endif()

if(TEMPOLINK_BUILD_JUCE_CLIENT)
  add_subdirectory(Client/JuceApp)
endif()
