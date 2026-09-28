import QtQuick
import QtQuick.Layouts
import Gittyup

// Find text in the editor. 'findBar' is the C++ FindWidget.
Rectangle {
    id: root

    color: Theme.panel

    Connections {
        target: findBar

        function onFocusRequested() {
            field.forceActiveFocus()
            field.selectAll()
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: Theme.border
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 10
        spacing: 8

        Item { Layout.fillWidth: true }

        Text {
            visible: findBar.hitsText !== ""
            text: findBar.hitsText
            color: findBar.hasMatches ? Theme.textMuted : Theme.deleted
            font.pixelSize: 12
        }

        ActionButton {
            compact: true
            implicitWidth: 28
            implicitHeight: 28
            enabled: findBar.hasMatches
            icon: "chevron-up"
            tip: qsTr("Previous match")
            onClicked: findBar.previous()
        }

        ActionButton {
            compact: true
            implicitWidth: 28
            implicitHeight: 28
            enabled: findBar.hasMatches
            icon: "chevron-down"
            tip: qsTr("Next match")
            onClicked: findBar.next()
        }

        TextField {
            id: field

            Layout.preferredWidth: 260
            implicitHeight: 30
            leftPadding: 30
            placeholderText: qsTr("Find")
            text: findBar.searchText
            onTextEdited: findBar.search(text)
            onAccepted: findBar.next()
            Keys.onEscapePressed: findBar.hide()

            Icon {
                anchors.left: parent.left
                anchors.leftMargin: 9
                anchors.verticalCenter: parent.verticalCenter
                name: "search"
                size: 14
                color: Theme.textMuted
            }
        }

        PushButton {
            implicitHeight: 30
            text: qsTr("Done")
            onClicked: findBar.hide()
        }
    }
}
