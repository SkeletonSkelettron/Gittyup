import QtQuick
import QtQuick.Controls.Basic as Controls
import QtQuick.Layouts
import Gittyup

// Repository sidebar. 'sidebar' is the C++ SideBar that owns this view.
Rectangle {
    id: root

    // Keep in sync with ItemKind in SideBar.cpp.
    readonly property int kindHeader: 0
    readonly property int kindOpen: 1
    readonly property int kindRecent: 2
    readonly property int kindAccount: 3
    readonly property int kindAddAccount: 4
    readonly property int kindRemoteRepo: 5
    readonly property int kindError: 6
    readonly property int kindProgress: 7
    readonly property int kindEmpty: 8

    color: Theme.sidebar

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Item {
            Layout.fillWidth: true
            implicitHeight: 40

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 14
                anchors.right: buttons.left
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Repositories")
                color: Theme.text
                font.pixelSize: 13
                font.bold: true
                elide: Text.ElideRight
            }

            Row {
                id: buttons

                anchors.right: parent.right
                anchors.rightMargin: 6
                anchors.verticalCenter: parent.verticalCenter
                spacing: 2

                ActionButton {
                    compact: true
                    icon: "plus"
                    hasMenu: true
                    menuOnly: true
                    tip: qsTr("Clone, open, or add an account")
                    onMenuRequested: (x, y) => sidebar.showAddMenu(x, y)
                }

                ActionButton {
                    compact: true
                    icon: "more"
                    hasMenu: true
                    menuOnly: true
                    tip: qsTr("Options")
                    onMenuRequested: (x, y) => sidebar.showOptionsMenu(x, y)
                }
            }
        }

        TreeView {
            id: tree

            // Row of the last clicked item that isn't an open repository.
            property int selectedRow: -1

            function restoreExpansion() {
                const model = sidebar.model
                for (let i = 0; i < model.rowCount(); ++i) {
                    const section = model.index(i, 0)
                    const sectionRow = tree.rowAtIndex(section)
                    if (sectionRow < 0)
                        continue

                    if (!sidebar.isExpanded(section)) {
                        tree.collapse(sectionRow)
                        continue
                    }

                    tree.expand(sectionRow)
                    for (let j = 0; j < model.rowCount(section); ++j) {
                        const child = model.index(j, 0, section)
                        if (model.rowCount(child) === 0)
                            continue

                        const row = tree.rowAtIndex(child)
                        if (row < 0)
                            continue

                        if (sidebar.isExpanded(child))
                            tree.expand(row)
                        else
                            tree.collapse(row)
                    }
                }
            }

            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: sidebar.model
            boundsBehavior: Flickable.StopAtBounds
            columnWidthProvider: function () { return tree.width }
            onWidthChanged: forceLayout()

            Component.onCompleted: restoreExpansion()

            Connections {
                target: sidebar.model

                function onModelReset() {
                    tree.selectedRow = -1
                    Qt.callLater(tree.restoreExpansion)
                }
            }

            Controls.ScrollBar.vertical: Controls.ScrollBar {
                policy: Controls.ScrollBar.AsNeeded
                contentItem: Rectangle {
                    implicitWidth: 6
                    radius: 3
                    color: Theme.textMuted
                    opacity: parent.pressed ? 0.7 : parent.hovered ? 0.5 : 0.3
                }
            }

            delegate: Item {
                id: item

                required property TreeView treeView
                required property bool isTreeNode
                required property bool expanded
                required property bool hasChildren
                required property int depth
                required property int row
                required property int column
                required property var display
                required property var toolTip
                required property var kind
                required property var iconName
                required property var isCurrent
                required property var count
                required property var removable

                readonly property bool isHeader: kind === root.kindHeader
                readonly property bool highlighted: isCurrent === true
                readonly property bool selected: !highlighted && tree.selectedRow === row
                readonly property var modelIndex: treeView.index(row, column)

                function toggle() {
                    treeView.toggleExpanded(row)
                    sidebar.setExpanded(modelIndex, treeView.isExpanded(row))
                }

                implicitWidth: treeView.width
                implicitHeight: isHeader ? 32 : 28

                Rectangle {
                    anchors.fill: parent
                    anchors.leftMargin: 6
                    anchors.rightMargin: 6
                    radius: 5
                    visible: !item.isHeader
                    color: item.highlighted ? Theme.selected
                                            : mouse.containsMouse ? Theme.hover
                                            : item.selected ? Theme.pressed
                                                            : "transparent"

                    Rectangle {
                        visible: item.highlighted
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        width: 3
                        height: parent.height - 10
                        radius: 1.5
                        color: Theme.accent
                    }
                }

                MouseArea {
                    id: mouse

                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton | Qt.RightButton

                    onClicked: (event) => {
                        if (event.button === Qt.RightButton) {
                            const p = mapToItem(null, event.x, event.y)
                            sidebar.showContextMenu(item.modelIndex, p.x, p.y)
                            return
                        }

                        if (item.hasChildren && (item.isHeader || item.kind === root.kindAccount)) {
                            item.toggle()
                            return
                        }

                        tree.selectedRow = item.kind === root.kindOpen ? -1 : item.row
                        sidebar.activate(item.modelIndex)
                    }
                    onDoubleClicked: (event) => {
                        if (event.button === Qt.LeftButton && !item.isHeader)
                            sidebar.open(item.modelIndex)
                    }
                    onExited: host.hideToolTip()
                }

                Timer {
                    interval: 700
                    running: mouse.containsMouse && !mouse.pressed && item.toolTip
                    onTriggered: {
                        const p = item.mapToItem(null, 0, 0)
                        host.showToolTip(item.toolTip, p.x, p.y, item.width, item.height)
                    }
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: item.isHeader ? 10 : 16 + (item.depth - 1) * 16
                    anchors.rightMargin: 12
                    spacing: 6

                    // Expand chevron for headers and accounts.
                    Icon {
                        visible: item.isHeader || item.kind === root.kindAccount
                        opacity: item.hasChildren ? 1 : 0
                        name: "chevron-right"
                        size: item.isHeader ? 12 : 11
                        color: Theme.textMuted
                        rotation: item.expanded ? 90 : 0
                        Behavior on rotation { NumberAnimation { duration: 120 } }
                    }

                    Icon {
                        id: icon

                        visible: !item.isHeader && item.iconName !== ""
                        name: item.iconName
                        size: 16
                        color: item.kind === root.kindError ? Theme.badge
                               : item.highlighted ? Theme.accent : Theme.textMuted

                        RotationAnimator on rotation {
                            running: item.kind === root.kindProgress
                            loops: Animation.Infinite
                            from: 0
                            to: 360
                            duration: 900
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        text: item.display !== undefined ? item.display : ""
                        elide: Text.ElideMiddle
                        color: item.isHeader ? Theme.textMuted
                               : item.highlighted ? Theme.selectedText
                               : item.kind === root.kindEmpty ? Theme.textDisabled
                                                              : Theme.text
                        font.pixelSize: item.isHeader ? 11 : 13
                        font.bold: item.isHeader || item.highlighted
                        font.italic: item.kind === root.kindEmpty || item.kind === root.kindProgress
                        font.capitalization: item.isHeader ? Font.AllUppercase : Font.MixedCase
                        font.letterSpacing: item.isHeader ? 0.8 : 0
                    }

                    // Section item count.
                    Rectangle {
                        visible: item.isHeader && item.count > 0
                        implicitWidth: Math.max(implicitHeight, countLabel.implicitWidth + 10)
                        implicitHeight: 16
                        radius: implicitHeight / 2
                        color: Theme.hover

                        Text {
                            id: countLabel

                            anchors.centerIn: parent
                            text: item.count
                            color: Theme.textMuted
                            font.pixelSize: 10
                            font.bold: true
                        }
                    }

                    // Quick action shown on hover.
                    Icon {
                        visible: item.removable === true || item.kind === root.kindAddAccount
                        opacity: mouse.containsMouse || actionMouse.containsMouse ? 1 : 0
                        name: item.kind === root.kindAddAccount ? "plus" : "close"
                        size: 14
                        color: actionMouse.containsMouse ? Theme.text : Theme.textMuted

                        MouseArea {
                            id: actionMouse

                            anchors.fill: parent
                            anchors.margins: -4
                            hoverEnabled: true
                            onClicked: {
                                if (item.kind === root.kindAddAccount)
                                    sidebar.open(item.modelIndex)
                                else
                                    sidebar.remove(item.modelIndex)
                            }
                        }
                    }
                }
            }
        }
    }

    // Separate the sidebar from the repository view.
    Rectangle {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: 1
        color: Theme.border
    }
}
