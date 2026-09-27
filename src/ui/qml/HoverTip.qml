import QtQuick

// Shows a native tool tip for 'target' after hovering over it for a while.
// Assign 'hovered' from the target's MouseArea.
Timer {
    id: root

    property Item target
    property string text
    property bool hovered: false

    interval: 650
    running: hovered && text !== ""
    onTriggered: {
        const p = target.mapToItem(null, 0, 0)
        host.showToolTip(text, p.x, p.y, target.width, target.height)
    }
    onHoveredChanged: {
        if (!hovered)
            host.hideToolTip()
    }
}
