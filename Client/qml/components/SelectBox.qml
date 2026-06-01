import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TempoLink

ComboBox {
    id: control

    implicitHeight: 48
    font.family: Theme.uiFont
    font.pixelSize: 14

    contentItem: Text {
        leftPadding: 14
        rightPadding: 32
        text: control.displayText
        font: control.font
        color: Theme.ink
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        clip: true
    }

    background: Rectangle {
        radius: Theme.smallRadius
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#262b31" }
            GradientStop { position: 1.0; color: "#0f1217" }
        }
        border.width: 1
        border.color: control.visualFocus ? Theme.gold : "#6f7681"
    }

    indicator: Text {
        anchors.right: parent.right
        anchors.rightMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        text: "v"
        color: Theme.inkSoft
        font.family: Theme.monoFont
        font.pixelSize: 11
    }

    popup: Popup {
        y: control.height + 4
        width: control.width
        padding: 6
        background: Rectangle {
            radius: Theme.smallRadius
            color: Theme.panelRaised
            border.color: "#737b85"
            border.width: 1
        }

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
        }
    }

    delegate: ItemDelegate {
        width: control.width - 12
        padding: 10
        highlighted: control.highlightedIndex === index

        background: Rectangle {
            radius: Theme.smallRadius
            color: parent.highlighted ? "#2d3239" : "transparent"
        }

        contentItem: Text {
            text: modelData
            color: Theme.ink
            font.family: Theme.uiFont
            font.pixelSize: 13
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }
    }
}
