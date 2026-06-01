import QtQuick
import TempoLink

Item {
    id: root

    property int bars: 32
    property int seed: 1
    property color barColor: Theme.steel
    property real barOpacity: 1.0

    implicitHeight: 48

    Repeater {
        model: root.bars

        Rectangle {
            required property int index

            readonly property real phase: index * 0.52 + root.seed * 1.17
            readonly property real normalized: 0.28 + Math.abs(Math.sin(phase)) * 0.68
            readonly property real gap: 4
            readonly property real barWidth: (root.width - gap * (root.bars - 1)) / root.bars
            readonly property real barHeight: Math.max(10, root.height * normalized)

            x: index * (barWidth + gap)
            y: (root.height - barHeight) * 0.5
            width: Math.max(2, barWidth)
            height: barHeight
            radius: 2
            color: root.barColor
            opacity: root.barOpacity
        }
    }
}
