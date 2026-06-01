import QtQuick
import QtQuick.Controls
import TempoLink

Button {
    id: control

    property string variant: "accent"
    property bool compact: false

    implicitHeight: compact ? 38 : 50
    implicitWidth: compact ? 96 : 158

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
                    : control.variant === "ghost" ? "#787f89"
                    : Theme.gold
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: control.variant === "danger" ? "#de7d73"
                     : control.variant === "ghost" ? "#2b3037"
                     : control.down ? Theme.goldDeep : Theme.goldSoft
            }
            GradientStop {
                position: 1.0
                color: control.variant === "danger" ? Theme.danger
                     : control.variant === "ghost" ? "#15191f"
                     : control.down ? "#7f6934" : Theme.gold
            }
        }
        opacity: control.enabled ? 1.0 : 0.45

        Rectangle {
            anchors.fill: parent
            anchors.margins: 1
            radius: parent.radius - 1
            color: "transparent"
            border.color: control.variant === "accent" ? "#42fff1c3" : "#18ffffff"
            border.width: control.variant === "accent" ? 1 : 0
        }
    }
}
