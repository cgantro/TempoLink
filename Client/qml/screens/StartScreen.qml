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
                    text: qsTr("Sign in")
                    color: Theme.ink
                    font.family: Theme.displayFont
                    font.pixelSize: 34
                    font.weight: Font.DemiBold
                }

                Item { Layout.fillWidth: true }

                StatusBadge {
                    text: root.currentTab === 0 ? qsTr("LOGIN")
                         : root.currentTab === 1 ? qsTr("SIGN UP")
                         : "GOOGLE"
                    tone: "gold"
                }
            }

            CardPanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                Item {
                    anchors.fill: parent
                    anchors.margins: 18

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 18

                        Item {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            Rectangle {
                                anchors.fill: parent
                                radius: 18
                                color: "#111111"
                                border.color: "#2a2a2a"
                                border.width: 1
                            }

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 28
                                spacing: 18

                                StatusBadge {
                                    text: qsTr("DESKTOP")
                                    tone: "gold"
                                }

                                Text {
                                    text: "TempoLink"
                                    color: Theme.ink
                                    font.family: Theme.displayFont
                                    font.pixelSize: Math.max(54, Math.min(76, Math.round(root.width * 0.04)))
                                    font.weight: Font.DemiBold
                                }

                                Item { Layout.fillHeight: true }

                                CardPanel {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 168

                                    ColumnLayout {
                                        anchors.fill: parent
                                        anchors.margins: 18
                                        spacing: 14

                                        RowLayout {
                                            Layout.fillWidth: true

                                            Text {
                                                text: qsTr("Ready")
                                                color: Theme.ink
                                                font.family: Theme.uiFont
                                                font.pixelSize: 18
                                                font.weight: Font.DemiBold
                                            }

                                            Item { Layout.fillWidth: true }

                                            StatusBadge {
                                                text: root.currentTab === 0 ? qsTr("LOGIN")
                                                     : root.currentTab === 1 ? qsTr("SIGN UP")
                                                     : "GOOGLE"
                                                tone: "good"
                                            }
                                        }

                                        WaveformStrip {
                                            Layout.fillWidth: true
                                            Layout.preferredHeight: 56
                                            bars: 34
                                            seed: 8
                                            barColor: Theme.gold
                                            barOpacity: 0.78
                                        }

                                        RowLayout {
                                            Layout.fillWidth: true
                                            spacing: 12

                                            Rectangle {
                                                Layout.fillWidth: true
                                                Layout.preferredHeight: 44
                                                radius: 12
                                                color: "#0d0d0d"
                                                border.color: "#242424"
                                                border.width: 1

                                                Text {
                                                    anchors.centerIn: parent
                                                    text: qsTr("macOS / Windows")
                                                    color: Theme.inkSoft
                                                    font.family: Theme.uiFont
                                                    font.pixelSize: 13
                                                }
                                            }

                                            Rectangle {
                                                Layout.fillWidth: true
                                                Layout.preferredHeight: 44
                                                radius: 12
                                                color: "#0d0d0d"
                                                border.color: "#242424"
                                                border.width: 1

                                                Text {
                                                    anchors.centerIn: parent
                                                    text: qsTr("Low latency")
                                                    color: Theme.inkSoft
                                                    font.family: Theme.uiFont
                                                    font.pixelSize: 13
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
    }
}
