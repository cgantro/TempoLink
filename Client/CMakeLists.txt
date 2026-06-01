include(FetchContent)

FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG        asio-1-30-2
)

FetchContent_MakeAvailable(asio)

find_package(Qt6 6.10 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)

qt_standard_project_setup(REQUIRES 6.10)

set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

add_library(tempolink_core STATIC
    src/Audio/AudioEngine.cpp
    src/Audio/JitterBuffer.cpp
    src/Network/PeerManager.cpp
    src/Network/UdpSession.cpp
)

set_target_properties(tempolink_core PROPERTIES
    AUTOMOC OFF
    AUTOUIC OFF
    AUTORCC OFF
)

target_include_directories(tempolink_core
    PUBLIC
        include
        ${asio_SOURCE_DIR}/asio/include
)

target_compile_definitions(tempolink_core PUBLIC ASIO_STANDALONE)
target_compile_features(tempolink_core PUBLIC cxx_std_20)

target_link_libraries(tempolink_core
    PUBLIC
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_events
)

qt_add_executable(TempoLink
    WIN32
    MACOSX_BUNDLE
    src/Main.cpp
    include/tempolink/ui/facades/AppStateFacade.h
    include/tempolink/ui/facades/DeviceFacade.h
    include/tempolink/ui/facades/SessionFacade.h
    include/tempolink/ui/models/LobbyViewModel.h
    include/tempolink/ui/models/ParticipantListModel.h
    src/UI/facades/AppStateFacade.cpp
    src/UI/facades/DeviceFacade.cpp
    src/UI/facades/SessionFacade.cpp
    src/UI/models/LobbyViewModel.cpp
    src/UI/models/ParticipantListModel.cpp
)

set_target_properties(TempoLink PROPERTIES
    AUTOMOC ON
    AUTOUIC OFF
    AUTORCC ON
)

target_include_directories(TempoLink PRIVATE include)

target_link_libraries(TempoLink
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
        tempolink_core
)

qt_add_qml_module(TempoLink
    URI TempoLink
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/Theme.qml
        qml/components/AppShell.qml
        qml/components/AvatarChip.qml
        qml/components/CardPanel.qml
        qml/components/FieldInput.qml
        qml/components/MetricPill.qml
        qml/components/PrimaryButton.qml
        qml/components/ScreenHeader.qml
        qml/components/SegmentedTabs.qml
        qml/components/SelectBox.qml
        qml/components/StatusBadge.qml
        qml/components/WaveformStrip.qml
        qml/screens/AudioScreen.qml
        qml/screens/LobbyScreen.qml
        qml/screens/NetworkScreen.qml
        qml/screens/RoomScreen.qml
        qml/screens/StartScreen.qml
)

if(WIN32)
    set_target_properties(TempoLink PROPERTIES
        QT_QMLCACHEGEN_DIRECT_CALLS OFF
    )
endif()

qt_generate_deploy_qml_app_script(
    TARGET TempoLink
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(TARGETS TempoLink
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

install(SCRIPT ${deploy_script})

add_subdirectory(Tests)
