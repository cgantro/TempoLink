pragma Singleton
import QtQuick

QtObject {
    readonly property color desktopTop: "#0d1736"
    readonly property color desktopBottom: "#081126"
    readonly property color background: "#0a1430"
    readonly property color backgroundDeep: "#070f25"
    readonly property color panel: "#10214b"
    readonly property color panelRaised: "#183160"
    readonly property color panelAlt: "#132851"
    readonly property color ink: "#f6efdf"
    readonly property color inkSoft: "#e6dcc5"
    readonly property color inkFaint: "#c4b99d"
    readonly property color line: "#41f1e8cf"
    readonly property color lineStrong: "#70f1e8cf"
    readonly property color gold: "#cba14c"
    readonly property color goldDeep: "#a07e34"
    readonly property color steel: "#84a4d6"
    readonly property color good: "#7bb58d"
    readonly property color warn: "#d98a5a"
    readonly property color danger: "#d2675f"

    readonly property string displayFont: Qt.application.font.family
    readonly property string uiFont: Qt.application.font.family
    readonly property string monoFont: Qt.application.font.family

    readonly property int radius: 12
    readonly property int smallRadius: 8
    readonly property int borderWidth: 1
}
