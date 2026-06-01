import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TempoLink

Item {
    id: root

    property string currentScreen: "start"
    property bool showRail: false
    property string statusText: ""
    signal navigateRequested(string screenId)

    default property alias screenContent: screenHost.data

    Rectangle {
        anchors.fill: parent
        color: Theme.background
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 56
            color: Theme.backgroundDeep
            border.color: Theme.line
            border.width: 1

            Text {
                anchors.centerIn: parent
                text: "TEMPOLINK"
                color: Theme.inkSoft
                font.family: Theme.displayFont
                font.pixelSize: 16
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
                Layout.preferredWidth: root.showRail ? 96 : 0
                Layout.fillHeight: true
                color: Theme.backgroundDeep
                border.color: Theme.line
                border.width: root.showRail ? 1 : 0

                Column {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12

                    Text {
                        text: "◐"
                        color: Theme.gold
                        font.pixelSize: 22
                        anchors.horizontalCenter: parent.horizontalCenter
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
                            width: 64
                            height: 58
                            radius: 10
                            color: root.currentScreen === modelData.key ? "#1a2b4f" : "transparent"
                            border.color: root.currentScreen === modelData.key ? Theme.gold : "transparent"
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
                                    width: 16
                                    height: 16
                                    radius: 8
                                    color: root.currentScreen === modelData.key ? Theme.gold : Theme.inkFaint
                                    anchors.horizontalCenter: parent.horizontalCenter
                                }

                                Text {
                                    text: modelData.label
                                    color: root.currentScreen === modelData.key ? Theme.gold : Theme.inkFaint
                                    font.family: Theme.uiFont
                                    font.pixelSize: 10
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
                        font.pixelSize: 10
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
