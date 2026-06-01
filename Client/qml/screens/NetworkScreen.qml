import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TempoLink
import "../components"

Item {
    id: root

    readonly property int pagePadding: Math.max(18, Math.min(28, Math.round(width * 0.016)))
    readonly property int sidePanelWidth: width < 1360 ? 340 : 380
    readonly property var latencySeries: [11, 13, 14, 12, 15, 16, 14, 13, 12, 14, 18, 16, 15, 14, 12, 11, 13, 15, 16, 14]
    readonly property real latencyBarWidth: Math.max(12, Math.min(28,
        (latencyChart.width - Math.max(0, (latencySeries.length - 1) * latencyChart.spacing)) / Math.max(1, latencySeries.length)))

    Rectangle {
        anchors.fill: parent
        color: Theme.veil
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.pagePadding
        spacing: root.pagePadding

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: root.pagePadding

            CardPanel {
                Layout.fillHeight: true
                Layout.preferredWidth: root.sidePanelWidth
                Layout.minimumWidth: 320
                Layout.maximumWidth: 410
                raised: true

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 18
                    spacing: 14

                    Text {
                        text: qsTr("Network")
                        color: Theme.gold
                        font.family: Theme.displayFont
                        font.pixelSize: 24
                        font.weight: Font.DemiBold
                    }

                    CardPanel {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 146

                        Column {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 10

                            Text {
                                text: qsTr("연결 방식")
                                color: Theme.inkFaint
                                font.family: Theme.monoFont
                                font.pixelSize: 10
                            }

                            Text {
                                text: qsTr("직접 연결")
                                color: Theme.good
                                font.family: Theme.displayFont
                                font.pixelSize: 34
                                elide: Text.ElideRight
                            }

                        }
                    }

                    ListView {
                        id: peerList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        spacing: 10
                        model: participantModel

                        delegate: Rectangle {
                            id: peerCard
                            required property string name
                            required property string instrument
                            required property string region
                            required property int rtt

                            width: peerList.width
                            height: 64
                            radius: 12
                            color: "#12161b"
                            border.color: "#5f6772"
                            border.width: 1

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 12

                                AvatarChip {
                                    size: 34
                                    name: peerCard.name
                                    instrument: peerCard.instrument
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2

                                    Text {
                                        Layout.fillWidth: true
                                        text: peerCard.name
                                        color: Theme.ink
                                        font.family: Theme.uiFont
                                        font.pixelSize: 13
                                        font.weight: Font.DemiBold
                                        elide: Text.ElideRight
                                    }

                                    Text {
                                        Layout.fillWidth: true
                                        text: peerCard.region
                                        color: Theme.inkFaint
                                        font.family: Theme.monoFont
                                        font.pixelSize: 10
                                        elide: Text.ElideRight
                                    }
                                }

                                MetricPill {
                                    value: peerCard.rtt
                                }
                            }
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: root.pagePadding

                CardPanel {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 320
                    clip: true

                    Item {
                        anchors.fill: parent

                        MistyBackdrop {
                            anchors.fill: parent
                        }

                        Rectangle {
                            anchors.fill: parent
                            gradient: Gradient {
                                GradientStop { position: 0.0; color: "#00000000" }
                                GradientStop { position: 0.6; color: "#4d090c10" }
                                GradientStop { position: 1.0; color: "#c108090d" }
                            }
                        }

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 24
                            spacing: 14

                            RowLayout {
                                Layout.fillWidth: true

                                ScreenHeader {
                                    Layout.fillWidth: true
                                    eyebrow: "NETWORK"
                                    title: qsTr("연결 상태")
                                }

                                MetricPill {
                                    value: sessionFacade.roomRtt
                                    large: true
                                }
                            }

                            Text {
                                Layout.fillWidth: true
                                text: qsTr("%1 ms").arg(sessionFacade.roomRtt)
                                color: Theme.inkSoft
                                font.family: Theme.displayFont
                                font.pixelSize: 22
                            }

                            Item {
                                Layout.fillWidth: true
                                Layout.fillHeight: true

                                Row {
                                    id: latencyChart
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.bottom: parent.bottom
                                    anchors.bottomMargin: 18
                                    spacing: 8

                                    Repeater {
                                        model: latencySeries

                                        Rectangle {
                                            required property int modelData
                                            width: root.latencyBarWidth
                                            height: Math.max(36, modelData * 5)
                                            radius: 5
                                            anchors.bottom: parent.bottom
                                            color: modelData <= 15 ? Theme.good : modelData <= 30 ? Theme.gold : Theme.warn
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                Item { Layout.fillHeight: true }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            PrimaryButton {
                compact: true
                variant: "ghost"
                text: qsTr("오디오")
                onClicked: appState.navigate("audio")
            }

            PrimaryButton {
                compact: true
                variant: "ghost"
                text: qsTr("룸")
                onClicked: appState.navigate("room")
            }
        }
    }
}
