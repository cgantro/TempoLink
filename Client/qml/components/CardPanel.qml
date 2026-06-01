import QtQuick
import TempoLink

Rectangle {
    id: root

    property bool raised: false

    gradient: Gradient {
        GradientStop { position: 0.0; color: root.raised ? "#2a2f36" : Theme.panelRaised }
        GradientStop { position: 0.14; color: root.raised ? "#20242a" : "#23272e" }
        GradientStop { position: 1.0; color: root.raised ? Theme.panel : Theme.panelAlt }
    }
    radius: Theme.radius
    border.color: "#717884"
    border.width: Theme.borderWidth
    opacity: 0.98

    Rectangle {
        anchors.fill: parent
        anchors.margins: 1
        radius: parent.radius - 1
        color: "transparent"
        border.color: "#29ffffff"
        border.width: 1
    }
}
