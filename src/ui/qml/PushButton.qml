import QtQuick
import Gittyup

// A text button. 'primary' buttons are filled with the accent color.
Rectangle {
    id: root

    property string text
    property string icon
    property string tip
    property bool primary: false
    property bool danger: false

    signal clicked()

    readonly property color foreground: !enabled ? Theme.textDisabled
                                       : primary ? Theme.accentText
                                       : danger ? Theme.deleted : Theme.text

    implicitWidth: row.implicitWidth + 20
    implicitHeight: 28
    radius: 6
    opacity: enabled ? 1 : 0.6
    color: primary ? (mouse.pressed ? Qt.darker(Theme.accent, 1.15)
                                    : mouse.containsMouse ? Qt.lighter(Theme.accent, 1.1)
                                                          : Theme.accent)
                   : (mouse.pressed ? Theme.pressed
                                    : mouse.containsMouse ? Theme.hover : Theme.field)
    border.color: primary ? "transparent" : Theme.border

    MouseArea {
        id: mouse

        anchors.fill: parent
        hoverEnabled: true
        onClicked: root.clicked()
    }

    HoverTip {
        target: root
        text: root.tip
        hovered: mouse.containsMouse
    }

    Row {
        id: row

        anchors.centerIn: parent
        spacing: 6

        Icon {
            visible: root.icon !== ""
            anchors.verticalCenter: parent.verticalCenter
            name: root.icon
            size: 14
            color: root.foreground
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: root.text
            color: root.foreground
            font.pixelSize: 12
            font.bold: root.primary
        }
    }
}
