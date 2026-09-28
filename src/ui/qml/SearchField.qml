import QtQuick
import Gittyup

// The search field of the tool bar. 'search' is the C++ SearchField, which
// shows the completions and the advanced search below the field.
Rectangle {
    id: root

    // The field on the screen.
    function screenRect() {
        const p = root.mapToItem(null, 0, 0)
        const global = host.mapToGlobal(p.x, p.y)
        return Qt.rect(global.x, global.y, root.width, root.height)
    }

    function clear() {
        const r = root.screenRect()
        search.edit("", 0, r.x, r.y, r.width, r.height)
    }

    implicitWidth: 220
    implicitHeight: 28
    radius: 6
    enabled: search.enabled
    opacity: enabled ? 1 : 0.5
    color: Theme.field
    border.color: input.activeFocus ? Theme.accent
                                    : hover.hovered ? Theme.textMuted : Theme.border

    HoverHandler {
        id: hover
    }

    Icon {
        id: leading

        anchors.left: parent.left
        anchors.leftMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        name: "search"
        size: 14
        color: Theme.textMuted
    }

    TextInput {
        id: input

        anchors.left: leading.right
        anchors.leftMargin: 6
        anchors.right: clearButton.visible ? clearButton.left : advancedButton.left
        anchors.rightMargin: 4
        anchors.verticalCenter: parent.verticalCenter
        clip: true
        text: search.text
        color: Theme.text
        selectionColor: Theme.selected
        selectedTextColor: Theme.selectedText
        selectByMouse: true
        font.pixelSize: 12

        onTextEdited: {
            const r = root.screenRect()
            search.edit(text, cursorPosition, r.x, r.y, r.width, r.height)
        }
        onActiveFocusChanged: {
            if (!activeFocus)
                search.hideCompletions()
        }

        Keys.onUpPressed: (event) => event.accepted = search.moveCompletion(-1)
        Keys.onDownPressed: (event) => event.accepted = search.moveCompletion(1)
        Keys.onReturnPressed: (event) => event.accepted = search.acceptCompletion()
        Keys.onEnterPressed: (event) => event.accepted = search.acceptCompletion()
        Keys.onEscapePressed: {
            if (!search.hideCompletions())
                root.clear()
        }

        Connections {
            target: search

            function onCursorRequested(position) {
                input.cursorPosition = position
            }
        }

        Text {
            anchors.fill: parent
            verticalAlignment: Text.AlignVCenter
            visible: !input.text && !input.preeditText
            text: search.placeholderText
            color: Theme.textMuted
            font: input.font
            elide: Text.ElideRight
        }
    }

    component FieldButton: Rectangle {
        id: button

        property string icon
        property string tip

        signal clicked()

        anchors.verticalCenter: parent.verticalCenter
        width: 20
        height: 20
        radius: 4
        color: buttonMouse.pressed ? Theme.pressed
                                   : buttonMouse.containsMouse ? Theme.hover : "transparent"

        Icon {
            anchors.centerIn: parent
            name: button.icon
            size: 12
            color: buttonMouse.containsMouse ? Theme.text : Theme.textMuted
        }

        MouseArea {
            id: buttonMouse

            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.ArrowCursor
            onClicked: button.clicked()
            onExited: host.hideToolTip()
        }

        HoverTip {
            target: button
            text: button.tip
            hovered: buttonMouse.containsMouse
        }
    }

    FieldButton {
        id: clearButton

        anchors.right: advancedButton.left
        visible: input.text !== ""
        icon: "close"
        tip: qsTr("Clear")
        onClicked: root.clear()
    }

    FieldButton {
        id: advancedButton

        anchors.right: parent.right
        anchors.rightMargin: 4
        icon: "chevron-down"
        tip: qsTr("Advanced Search")
        onClicked: {
            const r = root.screenRect()
            search.showAdvanced(r.x, r.y, r.width, r.height)
        }
    }
}
