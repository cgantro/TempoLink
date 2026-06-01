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
    readonly property int latencyValueSize: Math.max(42, Math.min(64, Math.round(width * 0.044)))

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

            ScreenHeader {
                anchors.left: parent.left
                anchors.leftMargin: 28
                anchors.verticalCenter: parent.verticalCenter
                eyebrow: "AUDIO / INPUT + OUTPUT"
                title: qsTr("오디오 장치")
            }
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            columns: root.stacked ? 1 : 2
            rowSpacing: root.outerPadding
            columnSpacing: root.outerPadding

            ScrollView {
                id: audioScroll
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.column: 0
                Layout.row: 0
                Layout.leftMargin: root.outerPadding
                Layout.topMargin: root.outerPadding
                Layout.rightMargin: root.stacked ? root.outerPadding : 0
                Layout.bottomMargin: root.stacked ? 0 : root.outerPadding
                clip: true

                contentWidth: availableWidth

                Item {
                    width: Math.max(audioScroll.availableWidth, 0)
                    implicitHeight: audioCards.implicitHeight + root.outerPadding * 2

                    ColumnLayout {
                        id: audioCards
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: root.outerPadding
                        spacing: root.outerPadding

                        CardPanel {
                            Layout.fillWidth: true
                            implicitHeight: 200

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 20
                                spacing: 14

                                Text {
                                    text: qsTr("입력 장치")
                                    color: Theme.ink
                                    font.family: Theme.displayFont
                                    font.pixelSize: 24
                                }

                                SelectBox {
                                    Layout.fillWidth: true
                                    model: deviceFacade.inputDevices
                                    currentIndex: Math.max(0, deviceFacade.inputDevices.indexOf(deviceFacade.selectedInputDevice))
                                    onActivated: deviceFacade.selectInputDevice(currentText)
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
                            }
                        }

                        CardPanel {
                            Layout.fillWidth: true
                            implicitHeight: 168

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 20
                                spacing: 14

                                Text {
                                    text: qsTr("출력 장치")
                                    color: Theme.ink
                                    font.family: Theme.displayFont
                                    font.pixelSize: 24
                                }

                                SelectBox {
                                    Layout.fillWidth: true
                                    model: deviceFacade.outputDevices
                                    currentIndex: Math.max(0, deviceFacade.outputDevices.indexOf(deviceFacade.selectedOutputDevice))
                                    onActivated: deviceFacade.selectOutputDevice(currentText)
                                }

                                Text {
                                    text: qsTr("현재 장치로 모니터링 중입니다.")
                                    color: Theme.inkSoft
                                    font.family: Theme.uiFont
                                    font.pixelSize: 13
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
                        spacing: 14

                        Text {
                            text: qsTr("예상 오디오 지연")
                            color: Theme.steel
                            font.family: Theme.uiFont
                            font.pixelSize: 11
                            font.weight: Font.Medium
                        }

                        Text {
                            text: Number(deviceFacade.estimatedLatencyMs).toFixed(1) + " ms"
                            color: Theme.gold
                            font.family: Theme.displayFont
                            font.pixelSize: root.latencyValueSize
                            font.weight: Font.DemiBold
                        }

                        Text {
                            width: parent.width
                            wrapMode: Text.WordWrap
                            text: qsTr("ASIO / CoreAudio 기준으로 현재 디바이스 버퍼와 I/O 지연을 바탕으로 추정한 값입니다.")
                            color: Theme.inkFaint
                            font.family: Theme.uiFont
                            font.pixelSize: 12
                            lineHeight: 1.5
                        }
                    }
                }

                CardPanel {
                    Layout.fillWidth: true
                    implicitHeight: 120

                    Text {
                        anchors.fill: parent
                        anchors.margins: 18
                        wrapMode: Text.WordWrap
                        text: qsTr("오디오 엔진은 JUCE/C++를 유지하고, 이 화면은 상태 조회와 장치 선택만 담당합니다.")
                        color: Theme.inkSoft
                        font.family: Theme.uiFont
                        font.pixelSize: 13
                        lineHeight: 1.5
                    }
                }
            }
        }
    }
}
