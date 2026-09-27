import QtQuick
import QtQuick.Controls.Basic as Controls
import QtQuick.Layouts
import Gittyup

// Branches, remotes, tags, stashes and submodules of the repository.
// 'refsPanel' is the C++ RefsPanel.
Rectangle {
    id: root

    // Keep in sync with RefsModel::Kind.
    readonly property int kindHeader: 0
    readonly property int kindBranch: 1
    readonly property int kindRemoteGroup: 2
    readonly property int kindRemoteBranch: 3
    readonly property int kindTag: 4
    readonly property int kindStash: 5
    readonly property int kindSubmodule: 6

    // Keep in sync with RefsModel::Section.
    readonly property var sectionIcons: ["laptop", "cloud", "tag", "stash", "repo"]

    function iconFor(kind, section, head) {
        switch (kind) {
        case kindHeader: return sectionIcons[section]
        case kindBranch: return head ? "check" : "branch"
        case kindRemoteGroup: return "cloud"
        case kindRemoteBranch: return "branch"
        case kindTag: return "tag"
        case kindStash: return "stash"
        case kindSubmodule: return "repo"
        }
        return ""
    }

    color: Theme.panel

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        FilterField {
            Layout.fillWidth: true
            Layout.margins: 8
            placeholder: qsTr("Filter branches, tags...")
            onEdited: (text) => refsPanel.setFilter(text)
        }

        ListView {
            id: list

            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            model: refsPanel.model

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
                id: row

                required property int index
                required property int kind
                required property int section
                required property string name
                required property string toolTip
                required property int depth
                required property bool isHead
                required property bool isCurrent
                required property int ahead
                required property int behind
                required property int count
                required property bool expanded
                required property bool expandable

                readonly property bool isHeader: kind === root.kindHeader
                readonly property bool hovered: mouse.containsMouse || addMouse.containsMouse

                width: ListView.view.width
                height: isHeader ? 30 : 26

                Rectangle {
                    anchors.fill: parent
                    anchors.leftMargin: 4
                    anchors.rightMargin: 4
                    radius: 5
                    color: row.isCurrent ? Theme.selected
                                         : row.hovered ? Theme.hover : "transparent"
                }

                Rectangle {
                    visible: row.isHeader && row.index > 0
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    height: 1
                    color: Theme.border
                }

                MouseArea {
                    id: mouse

                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    onClicked: (event) => {
                        if (event.button === Qt.RightButton) {
                            const p = mapToItem(null, event.x, event.y)
                            refsPanel.showContextMenu(row.index, p.x, p.y)
                        } else {
                            refsPanel.activate(row.index)
                        }
                    }
                    onDoubleClicked: (event) => {
                        if (event.button === Qt.LeftButton && !row.isHeader)
                            refsPanel.open(row.index)
                    }
                }

                HoverTip {
                    target: row
                    text: row.isHeader ? "" : row.toolTip
                    hovered: mouse.containsMouse && !row.isHeader
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10 + Math.max(0, row.depth - 1) * 14
                    anchors.rightMargin: 12
                    spacing: 6

                    Icon {
                        visible: row.expandable
                        name: "chevron-right"
                        size: 11
                        color: Theme.textMuted
                        rotation: row.expanded ? 90 : 0
                        Behavior on rotation { NumberAnimation { duration: 120 } }
                    }

                    // Align leaves with the text of expandable rows.
                    Item {
                        visible: !row.expandable && row.depth > 0
                        implicitWidth: row.depth > 1 ? 11 : 0
                    }

                    Icon {
                        name: root.iconFor(row.kind, row.section, row.isHead)
                        size: row.isHeader ? 14 : 15
                        color: row.isHead ? Theme.accent
                               : row.isCurrent ? Theme.selectedText : Theme.textMuted
                    }

                    Text {
                        Layout.fillWidth: true
                        text: row.name
                        elide: Text.ElideMiddle
                        color: row.isHeader ? Theme.textMuted
                               : row.isCurrent ? Theme.selectedText : Theme.text
                        font.pixelSize: row.isHeader ? 11 : 13
                        font.bold: row.isHeader || row.isHead
                        font.capitalization: row.isHeader ? Font.AllUppercase : Font.MixedCase
                        font.letterSpacing: row.isHeader ? 0.8 : 0
                    }

                    // Commits ahead and behind of the upstream branch.
                    Row {
                        visible: row.ahead > 0 || row.behind > 0
                        spacing: 4

                        Text {
                            visible: row.behind > 0
                            text: "↓" + row.behind
                            color: Theme.behind
                            font.pixelSize: 11
                            font.bold: true
                        }

                        Text {
                            visible: row.ahead > 0
                            text: "↑" + row.ahead
                            color: Theme.ahead
                            font.pixelSize: 11
                            font.bold: true
                        }
                    }

                    // Count of a section or remote.
                    Rectangle {
                        visible: (row.isHeader || row.kind === root.kindRemoteGroup)
                                 && !(row.isHeader && row.hovered)
                        implicitWidth: Math.max(implicitHeight, countLabel.implicitWidth + 10)
                        implicitHeight: 16
                        radius: implicitHeight / 2
                        color: Theme.hover

                        Text {
                            id: countLabel

                            anchors.centerIn: parent
                            text: row.count
                            color: Theme.textMuted
                            font.pixelSize: 10
                            font.bold: true
                        }
                    }

                    // Add a branch, remote, tag, stash or submodule.
                    Icon {
                        visible: row.isHeader && row.hovered
                        name: "plus"
                        size: 14
                        color: addMouse.containsMouse ? Theme.accent : Theme.textMuted

                        MouseArea {
                            id: addMouse

                            anchors.fill: parent
                            anchors.margins: -4
                            hoverEnabled: true
                            onClicked: refsPanel.add(row.section)
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: 1
        color: Theme.border
    }
}
