include_guard(GLOBAL)

include(GNUInstallDirs)

function(tempolink_configure_install target_name)
  if(APPLE)
    set_target_properties(${target_name} PROPERTIES
      MACOSX_BUNDLE TRUE
      MACOSX_BUNDLE_GUI_IDENTIFIER "com.tempolink.client"
      MACOSX_BUNDLE_BUNDLE_NAME "TempoLink"
      MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
      MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    )
    install(TARGETS ${target_name} BUNDLE DESTINATION .)

    install(CODE "
      set(app_path \"\${CMAKE_INSTALL_PREFIX}/TempoLink JUCE Client.app\")

      set(search_dirs \"/usr/local/lib;/usr/lib\")
      if(EXISTS \"/opt/homebrew/lib\")
        list(APPEND search_dirs \"/opt/homebrew/lib\")
      endif()

      message(STATUS \"Fixing up bundle dependencies for \${app_path}\")
      include(BundleUtilities)
      fixup_bundle(\"\${app_path}\" \"\" \"\${search_dirs}\")
      "
    )
  elseif(UNIX AND NOT APPLE)
    install(TARGETS ${target_name} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  elseif(WIN32)
    install(TARGETS ${target_name}
      RUNTIME_DEPENDENCY_SET app_deps
      RUNTIME DESTINATION .
    )

    install(RUNTIME_DEPENDENCY_SET app_deps
      DESTINATION .
      PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-"
      POST_EXCLUDE_REGEXES ".*[\\\\/][Ss][Yy][Ss][Tt][Ee][Mm]32[\\\\/].*"
    )
  endif()
endfunction()
