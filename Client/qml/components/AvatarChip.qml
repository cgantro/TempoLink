import QtQuick
import TempoLink

Item {
    id: root

    property string name: ""
    property string instrument: ""
    property bool highlight: false
    property int size: 40

    width: size
    height: size

    Rectangle {
        anchors.fill: parent
        radius: width / 2
        border.width: 1
        border.color: root.highlight ? Theme.gold : Theme.lineStrong
        gradient: Gradient {
            GradientStop { position: 0.0; color: Theme.panelRaised }
            GradientStop { position: 1.0; color: Theme.panelAlt }
        }
    }

    Text {
        anchors.centerIn: parent
        text: root.name.length > 0 ? root.name[0] : "?"
        color: root.highlight ? Theme.gold : Theme.ink
        font.family: Theme.displayFont
        font.pixelSize: root.size * 0.42
        font.weight: Font.DemiBold
    }
}
