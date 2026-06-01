import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TempoLink

Rectangle {
    id: root

    property var model: []
    property int currentIndex: 0
    signal tabSelected(int index)

    radius: Theme.smallRadius
    gradient: Gradient {
        GradientStop { position: 0.0; color: "#252930" }
        GradientStop { position: 1.0; color: "#101319" }
    }
    border.color: "#6b727d"
    border.width: 1
    implicitHeight: 42

    RowLayout {
        anchors.fill: parent
        anchors.margins: 1
        spacing: 1

        Repeater {
            model: root.model

            Button {
                required property int index
                required property var modelData

                Layout.fillWidth: true
                Layout.fillHeight: true
                text: modelData
                font.family: Theme.uiFont
                font.pixelSize: 13
                font.weight: Font.DemiBold
                onClicked: root.tabSelected(index)

                contentItem: Text {
                    text: parent.text
                    font: parent.font
                    color: index === root.currentIndex ? "#16120a" : Theme.inkSoft
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    renderType: Text.NativeRendering
                }

                background: Rectangle {
                    radius: Theme.smallRadius - 1
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: index === root.currentIndex ? Theme.goldSoft : "transparent" }
                        GradientStop { position: 1.0; color: index === root.currentIndex ? Theme.gold : "transparent" }
                    }
                    border.color: index === root.currentIndex ? Theme.goldDeep : "#00000000"
                    border.width: index === root.currentIndex ? 1 : 0
                }
            }
        }
    }
}
