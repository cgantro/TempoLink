import QtQuick
import QtQuick.Controls
import TempoLink

TextField {
    id: control

    implicitHeight: 48
    color: Theme.ink
    placeholderTextColor: Theme.inkFaint
    font.family: Theme.uiFont
    font.pixelSize: 14
    selectByMouse: true
    leftPadding: 14
    rightPadding: 14
    topPadding: 12
    bottomPadding: 12

    background: Rectangle {
        radius: Theme.smallRadius
        color: Theme.background
        border.width: 1
        border.color: control.activeFocus ? Theme.gold : Theme.lineStrong
    }
}
