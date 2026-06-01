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
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#262b31" }
            GradientStop { position: 1.0; color: "#0f1217" }
        }
        border.width: 1
        border.color: control.activeFocus ? Theme.gold : "#6f7681"
    }
}
