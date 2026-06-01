import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TempoLink
import "."

Item {
    id: root

    property string currentScreen: "start"
    property bool showRail: false
    property string statusText: ""
    signal navigateRequested(string screenId)

    default property alias screenContent: screenHost.data

    MistyBackdrop {
        anchors.fill: parent
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 64
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#232323" }
                GradientStop { position: 0.24; color: Theme.chromeSoft }
                GradientStop { position: 1.0; color: Theme.chrome }
            }
            border.color: Theme.chromeEdge
            border.width: 1

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: "#96ffffff"
                opacity: 0.18
            }

            Text {
                anchors.centerIn: parent
                text: "TEMPOLINK"
                color: Theme.inkSoft
                font.family: Theme.displayFont
                font.pixelSize: 18
                font.weight: Font.DemiBold
            }

            Row {
                anchors.right: parent.right
                anchors.rightMargin: 24
                anchors.verticalCenter: parent.verticalCenter
                spacing: 10

                Rectangle {
                    width: 8
                    height: 8
                    radius: 4
                    color: Theme.good
                }

                Text {
                    text: root.statusText
                    visible: text.length > 0
                    color: Theme.inkSoft
                    font.family: Theme.uiFont
                    font.pixelSize: 12
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Rectangle {
                visible: root.showRail
                Layout.preferredWidth: root.showRail ? 122 : 0
                Layout.fillHeight: true
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#141414" }
                    GradientStop { position: 1.0; color: "#0a0a0a" }
                }
                border.color: "#2f2f2f"
                border.width: root.showRail ? 1 : 0

                Column {
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 14

                    Rectangle {
                        width: 70
                        height: 54
                        radius: 12
                        anchors.horizontalCenter: parent.horizontalCenter
                        color: "#15181d"
                        border.color: "#303030"

                        Text {
                            anchors.centerIn: parent
                            text: "◐"
                            color: Theme.gold
                            font.pixelSize: 22
                        }
                    }

                    Repeater {
                        model: [
                            { key: "lobby", label: "LOBBY" },
                            { key: "room", label: "ROOM" },
                            { key: "audio", label: "AUDIO" },
                            { key: "network", label: "LINK" }
                        ]

                        Rectangle {
                            required property var modelData
                            width: 82
                            height: 74
                            radius: 12
                            color: root.currentScreen === modelData.key ? "#1e1e1e" : "#101010"
                            border.color: root.currentScreen === modelData.key ? Theme.gold : "#2e2e2e"
                            border.width: 1
                            anchors.horizontalCenter: parent.horizontalCenter

                            MouseArea {
                                anchors.fill: parent
                                onClicked: root.navigateRequested(modelData.key)
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                            }

                            Column {
                                anchors.centerIn: parent
                                spacing: 4

                                Rectangle {
                                    width: 18
                                    height: 18
                                    radius: 9
                                    color: root.currentScreen === modelData.key ? Theme.gold : Theme.inkFaint
                                    anchors.horizontalCenter: parent.horizontalCenter
                                }

                                Text {
                                    text: modelData.label
                                    color: root.currentScreen === modelData.key ? Theme.gold : Theme.inkFaint
                                    font.family: Theme.uiFont
                                    font.pixelSize: 11
                                    font.weight: Font.DemiBold
                                    anchors.horizontalCenter: parent.horizontalCenter
                                }
                            }
                        }
                    }

                    Item { width: 1; height: 1; Layout.fillHeight: true }

                    Text {
                        text: "END"
                        color: Theme.inkFaint
                        font.family: Theme.monoFont
                        font.pixelSize: 11
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
            }

            Item {
                id: screenHost
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }
}
