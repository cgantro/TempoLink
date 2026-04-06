include_guard(GLOBAL)

function(tempolink_ensure_juce_available)
  find_package(JUCE CONFIG QUIET)

  if(TARGET juce::juce_core)
    return()
  endif()

  if(TEMPOLINK_JUCE_SOURCE_DIR AND EXISTS "${TEMPOLINK_JUCE_SOURCE_DIR}/CMakeLists.txt")
    add_subdirectory("${TEMPOLINK_JUCE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/_deps/juce-src")
    return()
  endif()

  if(TEMPOLINK_FETCH_JUCE)
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
    return()
  endif()

  message(FATAL_ERROR
    "JUCE was not found.\n"
    "Set JUCE_DIR/CMAKE_PREFIX_PATH, or set TEMPOLINK_JUCE_SOURCE_DIR, or enable TEMPOLINK_FETCH_JUCE."
  )
endfunction()
