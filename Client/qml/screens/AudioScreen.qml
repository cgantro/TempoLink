import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TempoLink
import "../components"

Item {
    id: root

    readonly property int pagePadding: Math.max(18, Math.min(28, Math.round(width * 0.016)))
    readonly property int browserWidth: width < 1360 ? 350 : 390
    readonly property int latencyValueSize: width < 1360 ? 64 : 76

    Rectangle {
        anchors.fill: parent
        color: Theme.veilSoft
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
                Layout.preferredWidth: root.browserWidth
                Layout.minimumWidth: 330
                Layout.maximumWidth: 410
                raised: true

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 18
                    spacing: 16

                    Text {
                        text: qsTr("Audio")
                        color: Theme.gold
                        font.family: Theme.displayFont
                        font.pixelSize: 24
                        font.weight: Font.DemiBold
                    }

                    CardPanel {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 190

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 12

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
                                Layout.fillWidth: true
                                text: deviceFacade.selectedInputDevice
                                color: Theme.inkFaint
                                font.family: Theme.uiFont
                                font.pixelSize: 12
                                elide: Text.ElideRight
                            }
                        }
                    }

                    CardPanel {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 190

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 12

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
                                Layout.fillWidth: true
                                text: deviceFacade.selectedOutputDevice
                                color: Theme.inkFaint
                                font.family: Theme.uiFont
                                font.pixelSize: 12
                                elide: Text.ElideRight
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
                                text: qsTr("입력 레벨")
                                color: Theme.inkFaint
                                font.family: Theme.monoFont
                                font.pixelSize: 11
                            }

                            Rectangle {
                                width: parent.width
                                height: 12
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
                                GradientStop { position: 0.55; color: "#570a0c10" }
                                GradientStop { position: 1.0; color: "#c408090e" }
                            }
                        }

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 24
                            spacing: 24

                            ColumnLayout {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                spacing: 10

                                Text {
                                    text: "AUDIO"
                                    color: Theme.gold
                                    font.family: Theme.monoFont
                                    font.pixelSize: 11
                                    font.weight: Font.DemiBold
                                }

                                Text {
                                    text: qsTr("오디오 장치")
                                    color: Theme.ink
                                    font.family: Theme.displayFont
                                    font.pixelSize: 58
                                    font.weight: Font.DemiBold
                                }

                                Text {
                                    Layout.fillWidth: true
                                    text: qsTr("%1 ms").arg(Number(deviceFacade.estimatedLatencyMs).toFixed(1))
                                    color: Theme.inkSoft
                                    font.family: Theme.displayFont
                                    font.pixelSize: 22
                                }

                                WaveformStrip {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 86
                                    bars: 56
                                    seed: 13
                                    barColor: Theme.good
                                }

                                Item { Layout.fillHeight: true }
                            }

                            ColumnLayout {
                                Layout.preferredWidth: 290
                                Layout.fillHeight: true
                                spacing: 14

                                CardPanel {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 172

                                    Column {
                                        anchors.fill: parent
                                        anchors.margins: 18
                                        spacing: 12

                                        Text {
                                            text: qsTr("Latency")
                                            color: Theme.steel
                                            font.family: Theme.monoFont
                                            font.pixelSize: 11
                                        }

                                        Text {
                                            text: Number(deviceFacade.estimatedLatencyMs).toFixed(1) + " ms"
                                            color: Theme.gold
                                            font.family: Theme.displayFont
                                            font.pixelSize: root.latencyValueSize
                                            font.weight: Font.DemiBold
                                            elide: Text.ElideRight
                                        }

                                    }
                                }

                                CardPanel {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 110

                                    Column {
                                        anchors.fill: parent
                                        anchors.margins: 18
                                        spacing: 8

                                        Text {
                                            text: qsTr("Driver")
                                            color: Theme.inkFaint
                                            font.family: Theme.monoFont
                                            font.pixelSize: 10
                                        }

                                        Text {
                                            width: parent.width
                                            text: qsTr("ASIO / CoreAudio")
                                            color: Theme.ink
                                            font.family: Theme.displayFont
                                            font.pixelSize: 24
                                            elide: Text.ElideRight
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
                text: qsTr("룸")
                onClicked: appState.navigate("room")
            }

            PrimaryButton {
                compact: true
                variant: "ghost"
                text: qsTr("연결")
                onClicked: appState.navigate("network")
            }
        }
    }
}
