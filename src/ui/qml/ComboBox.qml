import QtQuick
import QtQuick.Controls.Basic as Controls
import Gittyup

// A drop-down list. Items of an array model can have an 'icon' and a
// 'detail' shown after the text.
Controls.ComboBox {
    id: control

    implicitHeight: 32
    leftPadding: 10
    rightPadding: 30
    font.pixelSize: 13

    function itemData(index) {
        const items = control.model
        if (!items || index < 0 || items.length === undefined || index >= items.length)
            return null
        const item = items[index]
        return item && typeof item === "object" ? item : null
    }

    readonly property var selectedItem: itemData(currentIndex)

    background: Rectangle {
        radius: 6
        color: control.pressed ? Theme.pressed : Theme.field
        border.width: control.activeFocus ? 2 : 1
        border.color: control.activeFocus || control.popup.visible
                      ? Theme.accent : control.hovered ? Theme.textMuted : Theme.border
    }

    // A fixed implicit width, so the text doesn't widen the control.
    contentItem: Item {
        implicitWidth: 160
        implicitHeight: 20

        Icon {
            id: currentIcon

            visible: control.selectedItem !== null && !!control.selectedItem.icon
            anchors.verticalCenter: parent.verticalCenter
            name: control.selectedItem && control.selectedItem.icon ? control.selectedItem.icon : ""
            size: 14
            color: Theme.textMuted
        }

        Text {
            anchors.left: currentIcon.visible ? currentIcon.right : parent.left
            anchors.leftMargin: currentIcon.visible ? 8 : 0
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            text: control.displayText
            elide: Text.ElideRight
            color: control.enabled ? Theme.text : Theme.textDisabled
            font: control.font
        }
    }

    indicator: Icon {
        x: control.width - width - 10
        y: (control.height - height) / 2
        name: "chevron-down"
        size: 12
        color: Theme.textMuted
    }

    delegate: Controls.ItemDelegate {
        id: item

        required property int index
        required property var modelData

        readonly property var entry: typeof modelData === "object" ? modelData : null

        width: ListView.view ? ListView.view.width : control.width
        height: 30
        leftPadding: 10
        rightPadding: 10
        highlighted: control.highlightedIndex === index

        background: Rectangle {
            radius: 5
            color: item.highlighted ? Theme.hover : "transparent"
        }

        contentItem: Row {
            spacing: 8

            Icon {
                visible: item.entry !== null && !!item.entry.icon
                anchors.verticalCenter: parent.verticalCenter
                name: item.entry && item.entry.icon ? item.entry.icon : ""
                size: 14
                color: item.index === control.currentIndex ? Theme.accent : Theme.textMuted
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: item.entry ? item.entry[control.textRole || "text"] : item.modelData
                color: item.index === control.currentIndex ? Theme.accent : Theme.text
                font.pixelSize: 13
                font.weight: item.index === control.currentIndex ? Font.DemiBold : Font.Normal
            }

            Text {
                visible: item.entry !== null && !!item.entry.detail
                anchors.verticalCenter: parent.verticalCenter
                text: item.entry && item.entry.detail ? item.entry.detail : ""
                color: Theme.textMuted
                font.pixelSize: 12
            }
        }
    }

    popup: Controls.Popup {
        y: control.height + 4
        width: control.width
        implicitHeight: Math.min(contentItem.implicitHeight + 8, 320)
        padding: 4

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
            boundsBehavior: Flickable.StopAtBounds

            Controls.ScrollBar.vertical: ThinScrollBar { thickness: 6 }
        }

        background: Rectangle {
            radius: 8
            color: Theme.panel
            border.color: Theme.border
        }
    }
}
