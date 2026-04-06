include_guard(GLOBAL)

function(tempolink_apply_common_target_settings target_name target_source_dir)
  target_compile_features(${target_name} PRIVATE cxx_std_20)

  target_include_directories(
    ${target_name}
    PRIVATE
      "${target_source_dir}/include"
      "${CMAKE_SOURCE_DIR}/Cpp/Client/JuceApp/include"
  )

  target_compile_definitions(
    ${target_name}
    PRIVATE
      JUCE_WEB_BROWSER=0
      JUCE_USE_CURL=0
  )
endfunction()
