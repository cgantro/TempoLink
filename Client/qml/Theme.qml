pragma Singleton
import QtQuick

QtObject {
    readonly property color desktopTop: "#141414"
    readonly property color desktopMid: "#111111"
    readonly property color desktopBottom: "#090909"
    readonly property color background: "#dd111111"
    readonly property color backgroundDeep: "#f00c0c0c"
    readonly property color panel: "#ef171717"
    readonly property color panelRaised: "#f01d1d1d"
    readonly property color panelAlt: "#ea121212"
    readonly property color panelInset: "#f0080808"
    readonly property color veil: "#7d080808"
    readonly property color veilSoft: "#4a090909"
    readonly property color chrome: "#222222"
    readonly property color chromeSoft: "#2d2d2d"
    readonly property color chromeEdge: "#3f3f3f"
    readonly property color ink: "#f4f2ee"
    readonly property color inkSoft: "#d8d3ca"
    readonly property color inkFaint: "#8f897f"
    readonly property color line: "#2ecfb54d"
    readonly property color lineStrong: "#50ffffff"
    readonly property color gold: "#c8a84c"
    readonly property color goldSoft: "#d6ba6f"
    readonly property color goldDeep: "#7e6730"
    readonly property color steel: "#a6a19a"
    readonly property color forestGlow: "#ece9df"
    readonly property color good: "#8ab18c"
    readonly property color warn: "#d19a62"
    readonly property color danger: "#cc6d61"

    readonly property string displayFont: Qt.platform.os === "osx" ? "SF Pro Display"
                                           : Qt.platform.os === "windows" ? "Segoe UI Variable Display"
                                           : Qt.application.font.family
    readonly property string uiFont: Qt.platform.os === "osx" ? "SF Pro Text"
                                      : Qt.platform.os === "windows" ? "Segoe UI Variable Text"
                                      : Qt.application.font.family
    readonly property string monoFont: Qt.platform.os === "osx" ? "SF Mono"
                                        : Qt.platform.os === "windows" ? "Consolas"
                                        : Qt.application.font.family

    readonly property int radius: 14
    readonly property int smallRadius: 8
    readonly property int borderWidth: 1
}
