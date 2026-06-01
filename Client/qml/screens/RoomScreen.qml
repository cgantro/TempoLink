import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TempoLink
import "../components"

Item {
    id: root

    readonly property int pagePadding: Math.max(16, Math.min(24, Math.round(width * 0.014)))
    readonly property int leftPanelWidth: width < 1320 ? 260 : 290
    readonly property int rightPanelWidth: width < 1320 ? 300 : 340
    readonly property int stageTitleSize: width < 1320 ? 52 : 62

    property var chatSeed: [
        { who: "윤도현", text: "다들 모니터 레벨 괜찮아요?", mine: false, time: "21:03" },
        { who: "Moonlight", text: "인트로부터 가면 됩니다.", mine: true, time: "21:04" },
        { who: "서린", text: "하나 둘 셋 넷.", mine: false, time: "21:04" }
    ]

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
                Layout.preferredWidth: root.leftPanelWidth
                Layout.minimumWidth: 250
                Layout.maximumWidth: 300
                raised: true

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 18
                    spacing: 16

                    Text {
                        text: qsTr("Me")
                        color: Theme.gold
                        font.family: Theme.displayFont
                        font.pixelSize: 24
                        font.weight: Font.DemiBold
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 14

                        AvatarChip {
                            size: 64
                            name: sessionFacade.meName
                            instrument: sessionFacade.meInstrument
                            highlight: true
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2

                            Text {
                                Layout.fillWidth: true
                                text: sessionFacade.meName
                                color: Theme.ink
                                font.family: Theme.displayFont
                                font.pixelSize: 28
                                elide: Text.ElideRight
                            }

                            Text {
                                Layout.fillWidth: true
                                text: sessionFacade.meInstrument + " / " + sessionFacade.meRegion
                                color: Theme.inkFaint
                                font.family: Theme.monoFont
                                font.pixelSize: 11
                                elide: Text.ElideRight
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 12
                        radius: 6
                        color: Theme.panelInset
                        border.color: "#5b626d"

                        Rectangle {
                            width: Math.max(parent.width * 0.08, parent.width * deviceFacade.inputLevel)
                            height: parent.height
                            radius: parent.radius
                            color: Theme.gold
                        }
                    }

                    CardPanel {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 168

                        Column {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 10

                            Text {
                                text: qsTr("Room")
                                color: Theme.ink
                                font.family: Theme.displayFont
                                font.pixelSize: 24
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
                                color: Theme.gold
                                font.family: Theme.monoFont
                                font.pixelSize: 12
                            }
                        }
                    }

                    StatusBadge {
                        text: sessionFacade.roomLive ? qsTr("LIVE") : qsTr("READY")
                        tone: sessionFacade.roomLive ? "good" : "gold"
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: root.pagePadding

                CardPanel {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 300
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
                                GradientStop { position: 0.52; color: "#48090c10" }
                                GradientStop { position: 1.0; color: "#bd08090e" }
                            }
                        }

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 24
                            spacing: 12

                            RowLayout {
                                Layout.fillWidth: true

                                Text {
                                    Layout.fillWidth: true
                                    text: sessionFacade.roomLive ? "ROOM / LIVE" : "ROOM / READY"
                                    color: Theme.gold
                                    font.family: Theme.monoFont
                                    font.pixelSize: 11
                                    font.weight: Font.DemiBold
                                }

                                MetricPill {
                                    value: sessionFacade.roomRtt
                                    large: true
                                }
                            }

                            Text {
                                Layout.fillWidth: true
                                text: sessionFacade.roomName
                                color: Theme.ink
                                font.family: Theme.displayFont
                                font.pixelSize: root.stageTitleSize
                                font.weight: Font.DemiBold
                                elide: Text.ElideRight
                                maximumLineCount: 1
                            }

                                Text {
                                    Layout.fillWidth: true
                                    text: qsTr("%1 · %2 BPM · %3 ms")
                                      .arg(sessionFacade.roomHost)
                                      .arg(sessionFacade.roomBpm)
                                      .arg(sessionFacade.roomRtt)
                                    color: Theme.inkSoft
                                    font.family: Theme.uiFont
                                    font.pixelSize: 16
                                    elide: Text.ElideRight
                                }

                            WaveformStrip {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 86
                                bars: 54
                                seed: 11
                                barColor: sessionFacade.roomLive ? Theme.good : Theme.steel
                            }

                            Item { Layout.fillHeight: true }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 12

                                StatusBadge {
                                    text: sessionFacade.roomRegion
                                    tone: "gold"
                                }

                                StatusBadge {
                                    text: qsTr("%1 BPM").arg(sessionFacade.roomBpm)
                                    tone: "good"
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
                                    text: qsTr("연결 상태")
                                    onClicked: appState.navigate("network")
                                }
                            }
                        }
                    }
                }

                CardPanel {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 12

                        RowLayout {
                            Layout.fillWidth: true

                            Text {
                                text: qsTr("참가자")
                                color: Theme.ink
                                font.family: Theme.displayFont
                                font.pixelSize: 24
                            }

                            Item { Layout.fillWidth: true }

                            Text {
                                text: qsTr("지역 / RTT")
                                color: Theme.inkFaint
                                font.family: Theme.monoFont
                                font.pixelSize: 10
                            }
                        }

                        ListView {
                            id: participantList
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true
                            spacing: 10
                            model: participantModel

                            delegate: Rectangle {
                                id: participantCard
                                required property int index
                                required property string name
                                required property string instrument
                                required property string region
                                required property int rtt

                                width: participantList.width
                                height: 86
                                radius: 12
                                color: "#12161b"
                                border.color: "#616975"
                                border.width: 1

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 14
                                    spacing: 14

                                    AvatarChip {
                                        size: 42
                                        name: participantCard.name
                                        instrument: participantCard.instrument
                                    }

                                    ColumnLayout {
                                        Layout.preferredWidth: 150
                                        spacing: 2

                                        Text {
                                            Layout.fillWidth: true
                                            text: participantCard.name
                                            color: Theme.ink
                                            font.family: Theme.uiFont
                                            font.pixelSize: 15
                                            font.weight: Font.DemiBold
                                            elide: Text.ElideRight
                                        }

                                        Text {
                                            Layout.fillWidth: true
                                            text: participantCard.instrument
                                            color: Theme.inkFaint
                                            font.family: Theme.monoFont
                                            font.pixelSize: 10
                                            elide: Text.ElideRight
                                        }
                                    }

                                    WaveformStrip {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 34
                                        bars: 40
                                        seed: index + 5
                                    }

                                    ColumnLayout {
                                        Layout.alignment: Qt.AlignRight
                                        spacing: 4

                                        Text {
                                            text: participantCard.region
                                            color: Theme.inkSoft
                                            font.family: Theme.monoFont
                                            font.pixelSize: 10
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
            }

            CardPanel {
                Layout.fillHeight: true
                        Layout.preferredWidth: root.rightPanelWidth
                Layout.minimumWidth: 280
                Layout.maximumWidth: 360
                raised: true

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 18
                    spacing: 12

                    Text {
                        text: qsTr("채팅")
                        color: Theme.ink
                        font.family: Theme.displayFont
                        font.pixelSize: 24
                    }

                    ListView {
                        id: chatList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        spacing: 10
                        model: root.chatSeed

                        delegate: Rectangle {
                            required property var modelData

                            width: chatList.width
                            height: chatColumn.implicitHeight + 24
                            radius: 12
                            color: modelData.mine ? "#233148" : "#111419"
                            border.color: modelData.mine ? Theme.gold : "#5e6671"
                            border.width: 1

                            Column {
                                id: chatColumn
                                width: parent.width - 24
                                anchors.left: parent.left
                                anchors.leftMargin: 12
                                anchors.verticalCenter: parent.verticalCenter
                                spacing: 4

                                Text {
                                    width: parent.width
                                    text: modelData.who + " / " + modelData.time
                                    color: modelData.mine ? Theme.gold : Theme.inkFaint
                                    font.family: Theme.monoFont
                                    font.pixelSize: 10
                                    elide: Text.ElideRight
                                }

                                Text {
                                    width: parent.width
                                    text: modelData.text
                                    color: Theme.ink
                                    font.family: Theme.uiFont
                                    font.pixelSize: 13
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }
                    }

                    PrimaryButton {
                        Layout.fillWidth: true
                        variant: "ghost"
                        text: qsTr("메시지")
                    }
                }
            }
        }

        CardPanel {
            Layout.fillWidth: true
            Layout.preferredHeight: 78

            RowLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 14

                Text {
                    text: qsTr("BPM %1").arg(sessionFacade.roomBpm)
                    color: Theme.ink
                    font.family: Theme.displayFont
                    font.pixelSize: 30
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
