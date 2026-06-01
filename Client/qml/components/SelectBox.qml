import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TempoLink

ComboBox {
    id: control

    implicitHeight: 48
    font.family: Theme.uiFont
    font.pixelSize: 14

    contentItem: Text {
        leftPadding: 14
        rightPadding: 32
        text: control.displayText
        font: control.font
        color: Theme.ink
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        clip: true
    }

    background: Rectangle {
        radius: Theme.smallRadius
        color: Theme.background
        border.width: 1
        border.color: control.visualFocus ? Theme.gold : Theme.lineStrong
    }

    indicator: Text {
        anchors.right: parent.right
        anchors.rightMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        text: "v"
        color: Theme.inkSoft
        font.family: Theme.monoFont
        font.pixelSize: 11
    }

}
