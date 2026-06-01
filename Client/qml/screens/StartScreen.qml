import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TempoLink
import "../components"

Item {
    id: root

    property int currentTab: 0
    readonly property int pagePadding: Math.max(18, Math.min(28, Math.round(width * 0.016)))
    readonly property int authPaneWidth: Math.max(340, Math.min(390, Math.round(width * 0.28)))
    readonly property int previewCardWidth: width < 1440 ? 260 : 290

    Rectangle {
        anchors.fill: parent
        color: Theme.veilSoft
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: root.pagePadding
        spacing: root.pagePadding

        CardPanel {
            id: authPane

            property string loginEmail: ""
            property string loginPassword: ""
            property string signupName: ""
            property string signupEmail: ""
            property string signupPassword: ""

            Layout.fillHeight: true
            Layout.preferredWidth: root.authPaneWidth
            Layout.minimumWidth: 330
            Layout.maximumWidth: 410
            raised: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 16

                Text {
                    text: "TempoLink"
                    color: Theme.ink
                    font.family: Theme.displayFont
                    font.pixelSize: 30
                    font.weight: Font.DemiBold
                }

                SegmentedTabs {
                    Layout.fillWidth: true
                    model: [qsTr("로그인"), qsTr("회원가입"), "Google"]
                    currentIndex: root.currentTab
                    onTabSelected: root.currentTab = index
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    visible: root.currentTab === 0
                    spacing: 12

                    Text {
                        text: qsTr("로그인")
                        color: Theme.inkSoft
                        font.family: Theme.uiFont
                        font.pixelSize: 14
                    }

                    FieldInput {
                        Layout.fillWidth: true
                        placeholderText: "you@tempolink.app"
                        text: authPane.loginEmail
                        onTextChanged: authPane.loginEmail = text
                    }

                    FieldInput {
                        Layout.fillWidth: true
                        echoMode: TextInput.Password
                        placeholderText: qsTr("비밀번호")
                        text: authPane.loginPassword
                        onTextChanged: authPane.loginPassword = text
                    }

                    PrimaryButton {
                        Layout.fillWidth: true
                        text: qsTr("로그인")
                        onClicked: sessionFacade.login(authPane.loginEmail, authPane.loginPassword)
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    visible: root.currentTab === 1
                    spacing: 12

                    Text {
                        text: qsTr("회원가입")
                        color: Theme.inkSoft
                        font.family: Theme.uiFont
                        font.pixelSize: 14
                    }

                    FieldInput {
                        Layout.fillWidth: true
                        placeholderText: qsTr("이름")
                        text: authPane.signupName
                        onTextChanged: authPane.signupName = text
                    }

                    FieldInput {
                        Layout.fillWidth: true
                        placeholderText: "you@tempolink.app"
                        text: authPane.signupEmail
                        onTextChanged: authPane.signupEmail = text
                    }

                    FieldInput {
                        Layout.fillWidth: true
                        echoMode: TextInput.Password
                        placeholderText: qsTr("비밀번호")
                        text: authPane.signupPassword
                        onTextChanged: authPane.signupPassword = text
                    }

                    PrimaryButton {
                        Layout.fillWidth: true
                        text: qsTr("계정 만들기")
                        onClicked: sessionFacade.signup(authPane.signupName,
                                                        authPane.signupEmail,
                                                        authPane.signupPassword)
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    visible: root.currentTab === 2
                    spacing: 12

                    Text {
                        text: "Google"
                        color: Theme.inkSoft
                        font.family: Theme.uiFont
                        font.pixelSize: 14
                    }

                    PrimaryButton {
                        Layout.fillWidth: true
                        text: qsTr("Google로 계속하기")
                        onClicked: sessionFacade.continueWithGooglePrototype()
                    }
                }

                Item { Layout.fillHeight: true }

                Text {
                    Layout.fillWidth: true
                    text: sessionFacade.authMessage
                    color: Theme.inkFaint
                    font.family: Theme.uiFont
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                    visible: text.length > 0
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: root.pagePadding

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Text {
                    text: "TempoLink"
                    color: Theme.ink
                    font.family: Theme.displayFont
                    font.pixelSize: 34
                    font.weight: Font.DemiBold
                }

                Item { Layout.fillWidth: true }

                PrimaryButton {
                    compact: true
                    variant: "ghost"
                    text: qsTr("Lobby")
                    onClicked: sessionFacade.continueWithGooglePrototype()
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

                                width: root.previewCardWidth
                                height: 188
                                radius: 14
                                color: "#141414"
                                border.color: live ? Theme.gold : "#2f2f2f"
                                border.width: 1

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
                                        font.pixelSize: 28
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
                                        Layout.preferredHeight: 34
                                        bars: 22
                                        seed: index + 4
                                        barColor: Theme.gold
                                        barOpacity: 0.78
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
    }
}
