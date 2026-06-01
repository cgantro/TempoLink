import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TempoLink
import "../components"

Item {
    id: root

    readonly property int pagePadding: Math.max(18, Math.min(28, Math.round(width * 0.016)))
    readonly property int browserWidth: width < 1500 ? 320 : 350
    readonly property int detailWidth: width < 1500 ? 320 : 350
    readonly property int cardWidth: width < 1500 ? 270 : 300

    property string selectedName: ""
    property string selectedHost: ""
    property string selectedRegion: ""
    property int selectedBpm: 0
    property int selectedRtt: 0
    property bool selectedLive: false
    property int selectedMembers: 0
    property int selectedSourceIndex: 0

    function selectRoom(name, host, region, bpm, rtt, live, memberCount, sourceIndex) {
        selectedName = name
        selectedHost = host
        selectedRegion = region
        selectedBpm = bpm
        selectedRtt = rtt
        selectedLive = live
        selectedMembers = memberCount
        selectedSourceIndex = sourceIndex
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.veilSoft
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: root.pagePadding
        spacing: root.pagePadding

        CardPanel {
            Layout.fillHeight: true
            Layout.preferredWidth: root.browserWidth
            Layout.minimumWidth: 300
            Layout.maximumWidth: 360
            raised: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 14

                Text {
                    text: qsTr("Sessions")
                    color: Theme.ink
                    font.family: Theme.displayFont
                    font.pixelSize: 28
                    font.weight: Font.DemiBold
                }

                FieldInput {
                    Layout.fillWidth: true
                    placeholderText: qsTr("검색")
                    text: lobbyModel.searchQuery
                    onTextChanged: lobbyModel.searchQuery = text
                }

                SegmentedTabs {
                    Layout.fillWidth: true
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

                ListView {
                    id: browserList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 10
                    model: lobbyModel

                    delegate: Rectangle {
                        required property int index
                        required property string name
                        required property string host
                        required property string region
                        required property int bpm
                        required property int rtt
                        required property bool live
                        required property int memberCount
                        required property int sourceIndex

                        Component.onCompleted: {
                            if (index === 0 && root.selectedName.length === 0)
                                root.selectRoom(name, host, region, bpm, rtt, live, memberCount, sourceIndex)
                        }

                        width: browserList.width
                        height: 82
                        radius: 12
                        color: root.selectedSourceIndex === sourceIndex ? "#191919" : "#101010"
                        border.color: root.selectedSourceIndex === sourceIndex ? Theme.gold : "#2d2d2d"
                        border.width: 1

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.selectRoom(parent.name, parent.host, parent.region, parent.bpm, parent.rtt,
                                                       parent.live, parent.memberCount, parent.sourceIndex)
                        }

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 6

                            RowLayout {
                                Layout.fillWidth: true

                                Text {
                                    Layout.fillWidth: true
                                    text: name
                                    color: Theme.ink
                                    font.family: Theme.uiFont
                                    font.pixelSize: 17
                                    font.weight: Font.DemiBold
                                    elide: Text.ElideRight
                                }

                                MetricPill {
                                    value: rtt
                                }
                            }

                            Text {
                                Layout.fillWidth: true
                                text: host
                                color: Theme.inkFaint
                                font.family: Theme.uiFont
                                font.pixelSize: 12
                                elide: Text.ElideRight
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

            RowLayout {
                Layout.fillWidth: true

                Text {
                    text: qsTr("Lobby")
                    color: Theme.ink
                    font.family: Theme.displayFont
                    font.pixelSize: 34
                    font.weight: Font.DemiBold
                }

                Item { Layout.fillWidth: true }

                PrimaryButton {
                    compact: true
                    text: qsTr("입장")
                    onClicked: sessionFacade.joinRoom(root.selectedSourceIndex)
                }
            }

            CardPanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 18
                    clip: true
                    contentWidth: availableWidth

                    Flow {
                        width: parent.availableWidth
                        spacing: 14

                        Repeater {
                            model: lobbyModel

                            Rectangle {
                                required property int index
                                required property string name
                                required property string host
                                required property string region
                                required property int bpm
                                required property int rtt
                                required property bool live
                                required property int memberCount
                                required property int sourceIndex

                                width: root.cardWidth
                                height: 196
                                radius: 14
                                color: "#131313"
                                border.color: root.selectedSourceIndex === sourceIndex ? Theme.gold : "#2f2f2f"
                                border.width: 1

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: root.selectRoom(parent.name, parent.host, parent.region, parent.bpm, parent.rtt,
                                                               parent.live, parent.memberCount, parent.sourceIndex)
                                }

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 16
                                    spacing: 10

                                    RowLayout {
                                        Layout.fillWidth: true

                                        StatusBadge {
                                            text: live ? qsTr("LIVE") : qsTr("READY")
                                            tone: live ? "gold" : "good"
                                        }

                                        Item { Layout.fillWidth: true }

                                        MetricPill {
                                            value: rtt
                                        }
                                    }

                                    Text {
                                        Layout.fillWidth: true
                                        text: name
                                        color: Theme.ink
                                        font.family: Theme.displayFont
                                        font.pixelSize: 26
                                        font.weight: Font.DemiBold
                                        elide: Text.ElideRight
                                    }

                                    Text {
                                        Layout.fillWidth: true
                                        text: host
                                        color: Theme.inkFaint
                                        font.family: Theme.uiFont
                                        font.pixelSize: 13
                                        elide: Text.ElideRight
                                    }

                                    WaveformStrip {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 36
                                        bars: 24
                                        seed: index + 7
                                        barColor: Theme.gold
                                        barOpacity: 0.75
                                    }

                                    RowLayout {
                                        Layout.fillWidth: true

                                        Text {
                                            text: region
                                            color: Theme.inkSoft
                                            font.family: Theme.monoFont
                                            font.pixelSize: 11
                                        }

                                        Text {
                                            text: bpm + " BPM"
                                            color: Theme.inkSoft
                                            font.family: Theme.monoFont
                                            font.pixelSize: 11
                                        }

                                        Item { Layout.fillWidth: true }

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
            }
        }

        CardPanel {
            Layout.fillHeight: true
            Layout.preferredWidth: root.detailWidth
            Layout.minimumWidth: 300
            Layout.maximumWidth: 360
            raised: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 14

                Text {
                    text: root.selectedName.length > 0 ? root.selectedName : qsTr("선택된 방")
                    color: Theme.ink
                    font.family: Theme.displayFont
                    font.pixelSize: 28
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                }

                StatusBadge {
                    text: root.selectedLive ? qsTr("LIVE") : qsTr("READY")
                    tone: root.selectedLive ? "gold" : "good"
                }

                CardPanel {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 140

                    Column {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 10

                        Text {
                            text: root.selectedHost
                            color: Theme.ink
                            font.family: Theme.uiFont
                            font.pixelSize: 18
                            font.weight: Font.DemiBold
                            elide: Text.ElideRight
                        }

                        Text {
                            text: root.selectedRegion + "  " + root.selectedBpm + " BPM"
                            color: Theme.inkFaint
                            font.family: Theme.monoFont
                            font.pixelSize: 11
                            elide: Text.ElideRight
                        }

                        MetricPill {
                            value: root.selectedRtt
                            large: true
                        }
                    }
                }

                CardPanel {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Column {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 10

                        Text {
                            text: qsTr("Members")
                            color: Theme.inkSoft
                            font.family: Theme.uiFont
                            font.pixelSize: 14
                        }

                        Text {
                            text: qsTr("%1명").arg(root.selectedMembers)
                            color: Theme.ink
                            font.family: Theme.displayFont
                            font.pixelSize: 42
                            font.weight: Font.DemiBold
                        }
                    }
                }

                PrimaryButton {
                    Layout.fillWidth: true
                    variant: "ghost"
                    text: qsTr("오디오")
                    onClicked: appState.navigate("audio")
                }
            }
        }
    }
}
