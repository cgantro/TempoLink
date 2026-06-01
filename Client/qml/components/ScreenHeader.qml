import QtQuick
import QtQuick.Layouts
import TempoLink

ColumnLayout {
    id: root

    property string eyebrow: ""
    property string title: ""

    Layout.fillWidth: true
    spacing: 6

    Text {
        Layout.fillWidth: true
        text: root.eyebrow
        color: Theme.gold
        font.family: Theme.uiFont
        font.pixelSize: 11
        font.weight: Font.Medium
        elide: Text.ElideRight
        renderType: Text.NativeRendering
    }

    Text {
        Layout.fillWidth: true
        text: root.title
        color: Theme.ink
        font.family: Theme.displayFont
        font.pixelSize: 40
        font.weight: Font.DemiBold
        elide: Text.ElideRight
        maximumLineCount: 1
        renderType: Text.NativeRendering
    }
}
