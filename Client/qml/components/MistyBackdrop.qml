import QtQuick
import TempoLink

Item {
    id: root

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#1a1a1a" }
            GradientStop { position: 0.25; color: "#121212" }
            GradientStop { position: 0.72; color: "#0c0c0c" }
            GradientStop { position: 1.0; color: "#070707" }
        }
    }

    Repeater {
        model: 8

        Rectangle {
            required property int index
            width: root.width * (0.08 + (index % 3) * 0.03)
            height: root.height * (0.22 + (index % 4) * 0.06)
            x: root.width * 0.06 + index * root.width * 0.11
            y: root.height * (0.18 + (index % 2) * 0.1)
            radius: 999
            color: index % 2 === 0 ? "#12d6ba6f" : "#10ffffff"
            opacity: 1.0
        }
    }

    Repeater {
        model: 3

        Rectangle {
            required property int index
            width: root.width * (0.42 + index * 0.1)
            height: root.height * (0.16 + index * 0.04)
            x: root.width * (0.12 + index * 0.08)
            y: root.height * (0.18 + index * 0.12)
            radius: height / 2
            color: index === 0 ? "#18ffffff" : "#10d7bf7b"
        }
    }

    Rectangle {
        width: root.width * 0.34
        height: root.height * 0.18
        anchors.right: parent.right
        anchors.rightMargin: root.width * 0.12
        anchors.bottom: parent.bottom
        anchors.bottomMargin: root.height * 0.14
        radius: height / 2
        color: "#14c8a84c"
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#00000000" }
            GradientStop { position: 0.7; color: "#09090920" }
            GradientStop { position: 1.0; color: "#b3070707" }
        }
    }
}
