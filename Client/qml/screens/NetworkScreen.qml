import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TempoLink
import "../components"

Item {
    id: root

    readonly property bool stacked: width < 1280
    readonly property int outerPadding: Math.max(18, Math.min(24, Math.round(width * 0.016)))
    readonly property int sidePanelWidth: Math.max(260, Math.min(360, Math.round(width * 0.28)))
    readonly property real latencyBarWidth: Math.max(10, Math.min(22,
        (latencyChart.width - Math.max(0, (latencySeries.length - 1) * latencyChart.spacing)) / Math.max(1, latencySeries.length)))

    Rectangle {
        anchors.fill: parent
        color: Theme.background
    }

    property var latencySeries: [11, 13, 14, 12, 15, 16, 14, 13, 12, 14, 18, 16, 15, 14, 12, 11, 13, 15, 16, 14]

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 112
            color: "transparent"
            border.color: Theme.line
            border.width: 1

            ScreenHeader {
                anchors.left: parent.left
                anchors.leftMargin: 28
                anchors.verticalCenter: parent.verticalCenter
                eyebrow: "NETWORK / LATENCY"
                title: qsTr("연결 상태")
            }
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            columns: root.stacked ? 1 : 2
            rowSpacing: root.outerPadding
            columnSpacing: root.outerPadding

            ColumnLayout {
                Layout.column: 0
                Layout.row: 0
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.leftMargin: root.outerPadding
                Layout.topMargin: root.outerPadding
                Layout.rightMargin: root.stacked ? root.outerPadding : 0
                Layout.bottomMargin: root.stacked ? 0 : root.outerPadding
                spacing: root.outerPadding

                CardPanel {
                    Layout.fillWidth: true
                    implicitHeight: 260

                    Column {
                        anchors.fill: parent
                        anchors.margins: 22
                        spacing: 16

                        Row {
                            spacing: 10
                            Text {
                                text: qsTr("실시간 지연")
                                color: Theme.ink
                                font.family: Theme.displayFont
                                font.pixelSize: 24
                            }
                            MetricPill {
                                value: sessionFacade.roomRtt
                            }
                        }

                        Row {
                            id: latencyChart
                            width: parent.width
                            spacing: 8
                            Repeater {
                                model: latencySeries

                                Rectangle {
                                    required property int modelData
                                    width: root.latencyBarWidth
                                    height: Math.max(26, modelData * 4)
                                    radius: 4
                                    color: modelData <= 15 ? Theme.good : modelData <= 30 ? Theme.gold : Theme.warn
                                    anchors.bottom: parent.bottom
                                }
                            }
                        }

                        Text {
                            text: qsTr("15ms 이하를 우선 목표로 두고, 방별 평균 RTT를 기준으로 화면을 정리합니다.")
                            color: Theme.inkFaint
                            font.family: Theme.uiFont
                            font.pixelSize: 12
                        }
                    }
                }

                CardPanel {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 10

                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                text: qsTr("피어 상태")
                                color: Theme.ink
                                font.family: Theme.displayFont
                                font.pixelSize: 22
                            }
                            Item { Layout.fillWidth: true }
                            Text {
                                text: qsTr("RTT / Region")
                                color: Theme.inkFaint
                                font.family: Theme.monoFont
                                font.pixelSize: 10
                            }
                        }

                        Repeater {
                            model: participantModel

                            Rectangle {
                                id: peerCard
                                required property string name
                                required property string instrument
                                required property string region
                                required property int rtt

                                Layout.fillWidth: true
                                implicitHeight: 54
                                radius: 10
                                color: Theme.backgroundDeep
                                border.color: Theme.line
                                border.width: 1

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 12

                                    AvatarChip {
                                        size: 30
                                        name: peerCard.name
                                    }

                                    Column {
                                        spacing: 2
                                        Text {
                                            text: peerCard.name
                                            color: Theme.ink
                                            font.family: Theme.uiFont
                                            font.pixelSize: 13
                                            font.weight: Font.DemiBold
                                        }
                                        Text {
                                            text: peerCard.region
                                            color: Theme.inkFaint
                                            font.family: Theme.monoFont
                                            font.pixelSize: 10
                                        }
                                    }

                                    Item { Layout.fillWidth: true }

                                    MetricPill {
                                        value: peerCard.rtt
                                    }
                                }
                            }
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.column: root.stacked ? 0 : 1
                Layout.row: root.stacked ? 1 : 0
                Layout.preferredWidth: root.sidePanelWidth
                Layout.minimumWidth: 260
                Layout.maximumWidth: 360
                Layout.fillWidth: root.stacked
                Layout.fillHeight: !root.stacked
                Layout.topMargin: root.stacked ? 0 : root.outerPadding
                Layout.leftMargin: root.stacked ? root.outerPadding : 0
                Layout.rightMargin: root.outerPadding
                Layout.bottomMargin: root.outerPadding
                spacing: root.outerPadding

                CardPanel {
                    Layout.fillWidth: true
                    implicitHeight: 240

                    Column {
                        anchors.fill: parent
                        anchors.margins: 24
                        spacing: 12

                        Text {
                            text: qsTr("연결 방식")
                            color: Theme.steel
                            font.family: Theme.uiFont
                            font.pixelSize: 11
                            font.weight: Font.Medium
                        }

                        Text {
                            text: qsTr("직접 연결")
                            color: Theme.good
                            font.family: Theme.displayFont
                            font.pixelSize: 34
                        }

                        Text {
                            width: parent.width
                            wrapMode: Text.WordWrap
                            text: qsTr("중앙 미디어 릴레이를 두지 않고, 가능한 한 직접 P2P 세션을 유지하는 구조입니다.")
                            color: Theme.inkSoft
                            font.family: Theme.uiFont
                            font.pixelSize: 13
                            lineHeight: 1.5
                        }
                    }
                }

                CardPanel {
                    Layout.fillWidth: true
                    implicitHeight: 140

                    Column {
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 10

                        Text {
                            text: qsTr("요약")
                            color: Theme.ink
                            font.family: Theme.displayFont
                            font.pixelSize: 22
                        }

                        Text {
                            text: qsTr("지역 / %1").arg(sessionFacade.roomRegion)
                            color: Theme.inkSoft
                            font.family: Theme.monoFont
                            font.pixelSize: 12
                        }

                        Text {
                            text: qsTr("현재 룸 / %1").arg(sessionFacade.roomName)
                            color: Theme.inkSoft
                            font.family: Theme.monoFont
                            font.pixelSize: 12
                        }
                    }
                }
            }
        }
    }
}
