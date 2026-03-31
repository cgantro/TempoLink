find_path(OPUS_INCLUDE_DIR
  NAMES opus/opus.h
  DOC "Path to Opus headers"
)

find_library(OPUS_LIBRARY
  NAMES opus libopus opus_static
  DOC "Path to Opus library"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Opus
  REQUIRED_VARS OPUS_INCLUDE_DIR OPUS_LIBRARY
)

if(Opus_FOUND AND NOT TARGET Opus::opus)
  add_library(Opus::opus UNKNOWN IMPORTED)
  set_target_properties(Opus::opus PROPERTIES
    IMPORTED_LOCATION "${OPUS_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${OPUS_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(OPUS_INCLUDE_DIR OPUS_LIBRARY)

