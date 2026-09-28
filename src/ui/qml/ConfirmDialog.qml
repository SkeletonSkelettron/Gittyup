import QtQuick
import QtQuick.Controls.Basic as Controls
import QtQuick.Layouts
import Gittyup

// 'dialog' is the C++ ConfirmDialog.
DialogPage {
    title: dialog.title
    acceptText: dialog.acceptText
    danger: dialog.danger
    contentWidth: 400

    RowLayout {
        Layout.fillWidth: true
        spacing: 14

        Rectangle {
            Layout.alignment: Qt.AlignTop
            implicitWidth: 36
            implicitHeight: 36
            radius: 18
            color: Qt.rgba(accent.r, accent.g, accent.b, 0.16)

            readonly property color accent: dialog.danger ? Theme.deleted : Theme.accent

            Icon {
                anchors.centerIn: parent
                name: dialog.danger ? "warning" : "info"
                size: 18
                color: parent.accent
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6

            Text {
                Layout.fillWidth: true
                text: dialog.text
                wrapMode: Text.Wrap
                color: Theme.text
                font.pixelSize: 13
            }

            Text {
                Layout.fillWidth: true
                visible: dialog.informativeText !== ""
                text: dialog.informativeText
                wrapMode: Text.Wrap
                color: Theme.textMuted
                font.pixelSize: 12
            }
        }
    }

    Rectangle {
        Layout.fillWidth: true
        visible: dialog.detailedText !== ""
        implicitHeight: Math.min(details.implicitHeight + 16, 140)
        radius: 6
        color: Theme.base
        border.color: Theme.border
        clip: true

        Controls.ScrollView {
            id: detailsView

            anchors.fill: parent
            anchors.margins: 8

            // ScrollView doesn't place a custom scroll bar.
            Controls.ScrollBar.vertical: ThinScrollBar {
                parent: detailsView
                x: detailsView.width - width
                height: detailsView.height
                thickness: 6
            }

            Text {
                id: details

                text: dialog.detailedText
                color: Theme.textMuted
                font.family: Theme.monoFont
                font.pixelSize: 12
            }
        }
    }

    CheckBox {
        visible: dialog.checkText !== ""
        text: dialog.checkText
        checked: dialog.checked
        onToggled: dialog.checked = checked
    }
}
