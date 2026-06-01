import QtQuick
import TempoLink

Rectangle {
    id: root

    property string text: ""
    property string tone: "gold"

    readonly property color toneColor: tone === "good" ? Theme.good
                                     : tone === "warn" ? Theme.warn
                                     : tone === "danger" ? Theme.danger
                                     : Theme.gold

    radius: 999
    color: "transparent"
    border.color: toneColor
    border.width: 1
    implicitHeight: 28
    implicitWidth: label.implicitWidth + 28

    Row {
        anchors.centerIn: parent
        spacing: 6

        Rectangle {
            width: 7
            height: 7
            radius: 4
            color: root.toneColor
        }

        Text {
            id: label
            text: root.text
            color: root.toneColor
            font.family: Theme.monoFont
            font.pixelSize: 11
            font.weight: Font.DemiBold
        }
    }
}
