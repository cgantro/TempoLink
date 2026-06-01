import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TempoLink
import "../components"

Item {
    Rectangle {
        anchors.fill: parent
        color: Theme.background
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 164
            color: "transparent"
            border.color: Theme.line
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 28
                spacing: 18

                RowLayout {
                    Layout.fillWidth: true

                    ScreenHeader {
                        eyebrow: "LOBBY / ACTIVE SESSIONS"
                        title: qsTr("오늘의 합주실")
                    }

                    Item { Layout.fillWidth: true }

                    PrimaryButton {
                        text: qsTr("첫 방으로 입장")
                        onClicked: sessionFacade.joinRoom(0)
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16

                    FieldInput {
                        Layout.preferredWidth: 360
                        placeholderText: qsTr("룸 또는 호스트 검색")
                        text: lobbyModel.searchQuery
                        onTextChanged: lobbyModel.searchQuery = text
                    }

                    SegmentedTabs {
                        Layout.preferredWidth: 420
                        model: ["All", "Live", "Idle", "KR", "JP"]
                        currentIndex: lobbyModel.statusFilter === "Live" ? 1
                                    : lobbyModel.statusFilter === "Idle" ? 2
                                    : lobbyModel.statusFilter === "KR" ? 3
                                    : lobbyModel.statusFilter === "JP" ? 4
                                    : 0
                        onTabSelected: {
                            const filters = ["All", "Live", "Idle", "KR", "JP"]
                            lobbyModel.statusFilter = filters[index]
                        }
                    }
                }
            }
        }

        GridView {
            id: grid
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: 24
            Layout.rightMargin: 24
            Layout.topMargin: 24
            Layout.bottomMargin: 24
            clip: true
            cellWidth: Math.max(420, width / 2 - 16)
            cellHeight: 232
            model: lobbyModel

            delegate: CardPanel {
                required property int index
                required property string name
                required property string host
                required property string region
                required property int bpm
                required property int rtt
                required property bool live
                required property int memberCount
                required property int sourceIndex

                width: grid.cellWidth - 18
                height: grid.cellHeight - 18

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: sessionFacade.joinRoom(sourceIndex)
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 18
                    spacing: 14

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        StatusBadge {
                            text: live ? qsTr("연주 중") : qsTr("대기")
                            tone: live ? "gold" : "good"
                        }

                        StatusBadge {
                            text: region
                            tone: "good"
                        }

                        Rectangle {
                            radius: 999
                            border.color: Theme.line
                            border.width: 1
                            color: "transparent"
                            implicitHeight: 28
                            implicitWidth: bpmLabel.implicitWidth + 20

                            Text {
                                id: bpmLabel
                                anchors.centerIn: parent
                                text: bpm + " BPM"
                                color: Theme.inkSoft
                                font.family: Theme.monoFont
                                font.pixelSize: 11
                            }
                        }
                    }

                    Text {
                        text: name
                        color: Theme.ink
                        font.family: Theme.displayFont
                        font.pixelSize: 28
                        font.weight: Font.DemiBold
                    }

                    Text {
                        text: "HOST / " + host
                        color: Theme.inkFaint
                        font.family: Theme.monoFont
                        font.pixelSize: 11
                    }

                    WaveformStrip {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 38
                        bars: 36
                        seed: index + 3
                        barColor: live ? Theme.steel : Theme.inkFaint
                        barOpacity: live ? 1.0 : 0.45
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Text {
                            text: qsTr("%1명 참여 중").arg(memberCount)
                            color: Theme.inkSoft
                            font.family: Theme.uiFont
                            font.pixelSize: 13
                        }

                        Item { Layout.fillWidth: true }

                        MetricPill {
                            value: rtt
                        }

                        PrimaryButton {
                            compact: true
                            text: qsTr("입장")
                            onClicked: sessionFacade.joinRoom(sourceIndex)
                        }
                    }
                }
            }
        }
    }
}
