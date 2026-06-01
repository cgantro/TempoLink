import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TempoLink
import "../components"

Item {
    id: root

    property int currentTab: 0
    readonly property int pagePadding: Math.max(28, Math.min(72, Math.round(width * 0.04)))
    readonly property int sectionSpacing: Math.max(16, Math.min(28, Math.round(width * 0.016)))
    readonly property int heroTitleSize: Math.max(60, Math.min(96, Math.round(width * 0.062)))
    readonly property int authPaneWidth: Math.max(360, Math.min(520, Math.round(width * 0.36)))

    Rectangle {
        anchors.fill: parent
        color: Theme.background
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Item {
            Layout.fillWidth: true
            Layout.preferredWidth: Math.max(root.width - root.authPaneWidth, root.width * 0.56)
            Layout.fillHeight: true

            ColumnLayout {
                anchors.fill: parent
                anchors.leftMargin: root.pagePadding
                anchors.rightMargin: root.pagePadding
                anchors.topMargin: root.pagePadding
                anchors.bottomMargin: root.pagePadding
                spacing: root.sectionSpacing

                Text {
                    text: "TempoLink"
                    color: Theme.gold
                    font.family: Theme.uiFont
                    font.pixelSize: 13
                    font.weight: Font.Medium
                }

                WaveformStrip {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Math.max(44, Math.min(56, Math.round(root.height * 0.06)))
                    bars: 48
                    seed: 7
                    barColor: Theme.steel
                }

                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    Rectangle {
                        width: Math.max(160, Math.min(parent.width * 0.34, 240))
                        height: width
                        radius: width / 2
                        anchors.right: parent.right
                        anchors.rightMargin: Math.max(root.pagePadding - 8, 24)
                        anchors.top: parent.top
                        anchors.topMargin: 12
                        color: Theme.panelRaised
                        border.color: Theme.lineStrong
                        border.width: 1

                        Rectangle {
                            width: parent.width * 0.76
                            height: width
                            radius: width / 2
                            anchors.centerIn: parent
                            anchors.horizontalCenterOffset: parent.width * 0.12
                            color: Theme.background
                        }
                    }

                    Column {
                        width: Math.min(parent.width * 0.8, 620)
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 14

                        Text {
                            text: qsTr("함께 연주할 준비가 끝났습니다.")
                            color: Theme.inkSoft
                            font.family: Theme.uiFont
                            font.pixelSize: Math.max(18, Math.min(22, Math.round(root.width * 0.015)))
                            font.weight: Font.Medium
                        }

                        Text {
                            text: "TempoLink"
                            color: Theme.ink
                            font.family: Theme.displayFont
                            font.pixelSize: root.heroTitleSize
                            font.weight: Font.DemiBold
                            renderType: Text.NativeRendering
                        }

                        Text {
                            width: parent.width
                            text: qsTr("로그인하고 바로 세션에 들어갈 수 있도록 시작 화면을 다시 정리했습니다.")
                            color: Theme.inkFaint
                            font.family: Theme.uiFont
                            font.pixelSize: Math.max(15, Math.min(18, Math.round(root.width * 0.0125)))
                            wrapMode: Text.WordWrap
                            lineHeight: 1.3
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: Theme.lineStrong
                    opacity: 0.7
                }
            }
        }

        Rectangle {
            id: authPane
            Layout.preferredWidth: root.authPaneWidth
            Layout.minimumWidth: 360
            Layout.maximumWidth: 520
            Layout.fillHeight: true
            color: Theme.backgroundDeep
            border.color: Theme.line
            border.width: 1

            property string loginEmail: ""
            property string loginPassword: ""
            property string signupName: ""
            property string signupEmail: ""
            property string signupPassword: ""

            ColumnLayout {
                width: parent.width - Math.max(56, Math.min(96, Math.round(parent.width * 0.18)))
                anchors.centerIn: parent
                spacing: root.sectionSpacing

                Text {
                    text: "ACCOUNT"
                    color: Theme.gold
                    font.family: Theme.uiFont
                    font.pixelSize: 12
                    font.weight: Font.Medium
                }

                Text {
                    text: root.currentTab === 0 ? qsTr("로그인")
                         : root.currentTab === 1 ? qsTr("회원가입")
                         : qsTr("Google로 계속하기")
                    color: Theme.ink
                    font.family: Theme.displayFont
                    font.pixelSize: Math.max(30, Math.min(40, Math.round(parent.width * 0.08)))
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
                    spacing: 14
                    visible: root.currentTab === 0

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
                    spacing: 14
                    visible: root.currentTab === 1

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
                    spacing: 14
                    visible: root.currentTab === 2

                    PrimaryButton {
                        Layout.fillWidth: true
                        text: qsTr("Google로 계속하기")
                        onClicked: sessionFacade.continueWithGooglePrototype()
                    }
                }

                Text {
                    Layout.fillWidth: true
                    text: sessionFacade.authMessage
                    color: Theme.inkSoft
                    font.family: Theme.uiFont
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                    visible: text.length > 0
                }
            }
        }
    }
}
