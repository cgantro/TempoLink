import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TempoLink
import "components"
import "screens"

ApplicationWindow {
    id: root

    width: 1440
    height: 920
    minimumWidth: 1120
    minimumHeight: 760
    visible: true
    title: "TempoLink"
    color: Theme.desktopBottom

    AppShell {
        anchors.fill: parent
        currentScreen: appState.currentScreen
        showRail: appState.showRail
        statusText: appState.connectionStatusText
        onNavigateRequested: appState.navigate(screenId)

        Loader {
            anchors.fill: parent
            sourceComponent: appState.currentScreen === "start" ? startScreen
                           : appState.currentScreen === "lobby" ? lobbyScreen
                           : appState.currentScreen === "room" ? roomScreen
                           : appState.currentScreen === "audio" ? audioScreen
                           : networkScreen
        }
    }

    Component { id: startScreen; StartScreen {} }
    Component { id: lobbyScreen; LobbyScreen {} }
    Component { id: roomScreen; RoomScreen {} }
    Component { id: audioScreen; AudioScreen {} }
    Component { id: networkScreen; NetworkScreen {} }
}
