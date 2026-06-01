import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TempoLink
import "../components"

Item {
    id: root

    readonly property int layoutMode: width < 1180 ? 1 : width < 1380 ? 2 : 3
    readonly property int outerPadding: Math.max(16, Math.min(24, Math.round(width * 0.016)))
    readonly property int leftPanelWidth: Math.max(240, Math.min(300, Math.round(width * 0.22)))
    readonly property int rightPanelWidth: Math.max(260, Math.min(340, Math.round(width * 0.24)))

    property var chatSeed: [
        { who: "윤도현", text: "다들 모니터 레벨 괜찮아요?", mine: false, time: "21:03" },
        { who: "Moonlight", text: "인트로부터 가면 됩니다.", mine: true, time: "21:04" },
        { who: "서린", text: "하나 둘 셋 넷.", mine: false, time: "21:04" }
    ]

    Rectangle {
        anchors.fill: parent
        color: Theme.background
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 112
            color: "transparent"
            border.color: Theme.line
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 24
                spacing: 16

                ScreenHeader {
                    eyebrow: sessionFacade.roomLive ? "ROOM / LIVE" : "ROOM / IDLE"
                    title: sessionFacade.roomName
                }

                Item { Layout.fillWidth: true }

                Column {
                    spacing: 2
                    Text {
                        text: qsTr("평균 지연")
                        color: Theme.inkFaint
                        font.family: Theme.monoFont
                        font.pixelSize: 10
                    }
                    MetricPill {
                        value: sessionFacade.roomRtt
                        large: true
                    }
                }
            }
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            columns: root.layoutMode
            rowSpacing: 0
            columnSpacing: 0

            Rectangle {
                Layout.row: 0
                Layout.column: 0
                Layout.columnSpan: 1
                Layout.preferredWidth: root.leftPanelWidth
                Layout.minimumWidth: 240
                Layout.maximumWidth: 300
                Layout.fillWidth: root.layoutMode !== 3
                Layout.fillHeight: root.layoutMode === 3
                Layout.preferredHeight: root.layoutMode === 3 ? -1 : (root.layoutMode === 2 ? 260 : 220)
                color: Theme.backgroundDeep
                border.color: Theme.line
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 22
                    spacing: 18

                    Text {
                        text: qsTr("내 채널")
                        color: Theme.gold
                        font.family: Theme.uiFont
                        font.pixelSize: 11
                        font.weight: Font.Medium
                    }

                    RowLayout {
                        spacing: 14

                        AvatarChip {
                            size: 58
                            name: sessionFacade.meName
                            instrument: sessionFacade.meInstrument
                            highlight: true
                        }

                        Column {
                            spacing: 3
                            Text {
                                text: sessionFacade.meName
                                color: Theme.ink
                                font.family: Theme.displayFont
                                font.pixelSize: 24
                            }
                            Text {
                                text: sessionFacade.meInstrument
                                color: Theme.inkFaint
                                font.family: Theme.uiFont
                                font.pixelSize: 12
                            }
                        }
                    }

                    Text {
                        text: qsTr("입력 레벨")
                        color: Theme.inkFaint
                        font.family: Theme.uiFont
                        font.pixelSize: 12
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 10
                        radius: 5
                        color: Theme.background
                        border.color: Theme.line

                        Rectangle {
                            width: parent.width * Math.max(0.04, deviceFacade.inputLevel)
                            height: parent.height
                            radius: 5
                            color: Theme.gold
                        }
                    }

                    CardPanel {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 132

                        Column {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 10

                            Text {
                                text: qsTr("세션 정보")
                                color: Theme.ink
                                font.family: Theme.displayFont
                                font.pixelSize: 20
                            }

                            Text {
                                text: qsTr("호스트 / %1").arg(sessionFacade.roomHost)
                                color: Theme.inkSoft
                                font.family: Theme.monoFont
                                font.pixelSize: 12
                            }

                            Text {
                                text: qsTr("지역 / %1").arg(sessionFacade.roomRegion)
                                color: Theme.inkSoft
                                font.family: Theme.monoFont
                                font.pixelSize: 12
                            }

                            Text {
                                text: qsTr("템포 / %1 BPM").arg(sessionFacade.roomBpm)
                                color: Theme.inkSoft
                                font.family: Theme.monoFont
                                font.pixelSize: 12
                            }
                        }
                    }
                }
            }

            ScrollView {
                id: participantScroll
                Layout.row: root.layoutMode === 1 ? 1 : 0
                Layout.column: root.layoutMode === 1 ? 0 : 1
                Layout.columnSpan: 1
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: root.layoutMode === 3 ? 420 : 0
                clip: true

                contentWidth: availableWidth

                Item {
                    width: Math.max(participantScroll.availableWidth, 0)
                    implicitHeight: participantCards.implicitHeight + root.outerPadding * 2

                    ColumnLayout {
                        id: participantCards
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: root.outerPadding
                        spacing: 14

                        Repeater {
                            model: participantModel

                            CardPanel {
                                id: participantCard
                                required property int index
                                required property string name
                                required property string instrument
                                required property string region
                                required property int rtt

                                Layout.fillWidth: true
                                Layout.preferredHeight: 92

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 16
                                    spacing: 16

                                    AvatarChip {
                                        size: 44
                                        name: participantCard.name
                                        instrument: participantCard.instrument
                                    }

                                    Column {
                                        Layout.preferredWidth: 120
                                        spacing: 3
                                        Text {
                                            width: parent.width
                                            text: participantCard.name
                                            color: Theme.ink
                                            font.family: Theme.uiFont
                                            font.pixelSize: 15
                                            font.weight: Font.DemiBold
                                            elide: Text.ElideRight
                                        }
                                        Text {
                                            width: parent.width
                                            text: participantCard.instrument + " / " + participantCard.region
                                            color: Theme.inkFaint
                                            font.family: Theme.monoFont
                                            font.pixelSize: 10
                                            elide: Text.ElideRight
                                        }
                                    }

                                    WaveformStrip {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 34
                                        bars: 44
                                        seed: participantCard.index + 4
                                    }

                                    MetricPill {
                                        value: participantCard.rtt
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.row: root.layoutMode === 3 ? 0 : (root.layoutMode === 2 ? 1 : 2)
                Layout.column:  root.layoutMode === 3 ? 2 : 0
                Layout.columnSpan: root.layoutMode === 2 ? 2 : 1
                Layout.preferredWidth: root.rightPanelWidth
                Layout.minimumWidth: 260
                Layout.maximumWidth: 340
                Layout.fillWidth: root.layoutMode !== 3
                Layout.fillHeight: root.layoutMode === 3
                Layout.preferredHeight: root.layoutMode === 3 ? -1 : (root.layoutMode === 2 ? 240 : 280)
                color: Theme.backgroundDeep
                border.color: Theme.line
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12

                    Text {
                        text: qsTr("채팅")
                        color: Theme.ink
                        font.family: Theme.displayFont
                        font.pixelSize: 22
                    }

                    ScrollView {
                        id: chatScroll
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        contentWidth: availableWidth

                        Item {
                            width: Math.max(chatScroll.availableWidth, 0)
                            implicitHeight: chatColumn.implicitHeight

                            ColumnLayout {
                                id: chatColumn
                                width: parent.width
                                spacing: 10

                                Repeater {
                                    model: chatSeed

                                    Rectangle {
                                        required property var modelData
                                        Layout.fillWidth: true
                                        implicitHeight: msg.implicitHeight + 26
                                        radius: 10
                                        color: modelData.mine ? "#20365a" : Theme.panel
                                        border.color: modelData.mine ? Theme.gold : Theme.line
                                        border.width: 1

                                        Column {
                                            anchors.fill: parent
                                            anchors.margins: 12
                                            spacing: 4
                                            Text {
                                                text: modelData.who + " / " + modelData.time
                                                color: modelData.mine ? Theme.gold : Theme.inkFaint
                                                font.family: Theme.monoFont
                                                font.pixelSize: 10
                                            }
                                            Text {
                                                id: msg
                                                width: parent.width
                                                wrapMode: Text.WordWrap
                                                text: modelData.text
                                                color: Theme.ink
                                                font.family: Theme.uiFont
                                                font.pixelSize: 13
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    PrimaryButton {
                        Layout.fillWidth: true
                        compact: false
                        variant: "ghost"
                        text: qsTr("메시지 입력은 다음 단계")
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 72
            color: Theme.backgroundDeep
            border.color: Theme.line
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 12

                Text {
                    text: qsTr("BPM %1").arg(sessionFacade.roomBpm)
                    color: Theme.ink
                    font.family: Theme.displayFont
                    font.pixelSize: 28
                }

                Item { Layout.fillWidth: true }

                PrimaryButton {
                    compact: true
                    variant: "ghost"
                    text: qsTr("오디오")
                    onClicked: appState.navigate("audio")
                }

                PrimaryButton {
                    compact: true
                    variant: "ghost"
                    text: qsTr("연결")
                    onClicked: appState.navigate("network")
                }

                PrimaryButton {
                    compact: true
                    variant: "danger"
                    text: qsTr("나가기")
                    onClicked: sessionFacade.leaveRoom()
                }
            }
        }
    }
}
