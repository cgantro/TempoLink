import QtQuick
import TempoLink

Text {
    id: root

    property int value: 0
    property bool large: false

    color: value <= 15 ? Theme.good : value <= 30 ? Theme.gold : Theme.warn
    font.family: Theme.monoFont
    font.pixelSize: large ? 28 : 13
    font.weight: Font.Bold
    text: large ? value + " ms" : value + "ms"
}
