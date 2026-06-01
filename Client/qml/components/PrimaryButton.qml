import QtQuick
import QtQuick.Controls
import TempoLink

Button {
    id: control

    property string variant: "accent"
    property bool compact: false

    implicitHeight: compact ? 36 : 48
    implicitWidth: compact ? 92 : 150

    font.family: Theme.uiFont
    font.pixelSize: compact ? 12 : 14
    font.weight: Font.DemiBold

    contentItem: Text {
        text: control.text
        color: control.variant === "accent" ? "#17130a" : Theme.ink
        font: control.font
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        renderType: Text.NativeRendering
    }

    background: Rectangle {
        radius: compact ? Theme.smallRadius : Theme.radius
        border.width: 1
        border.color: control.variant === "danger" ? Theme.danger
                    : control.variant === "ghost" ? Theme.lineStrong
                    : Theme.gold
        color: control.variant === "danger" ? Theme.danger
             : control.variant === "ghost" ? "transparent"
             : control.down ? Theme.goldDeep : Theme.gold
        opacity: control.enabled ? 1.0 : 0.45
    }
}
