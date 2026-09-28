import QtQuick
import QtQuick.Controls.Basic as Controls
import QtQuick.Layouts
import Gittyup

// The diff of the selected file. 'detailView.diff' is the C++ DiffModel.
Rectangle {
    id: root

    readonly property var diff: detailView.diff

    // Keep in sync with DiffModel::Kind and git::Index::StagedState.
    readonly property int hunkRow: 0
    readonly property int unstaged: 0
    readonly property int partiallyStaged: 1
    readonly property int staged: 2

    readonly property int lineHeight: 20
    readonly property int numberWidth: charWidth * diff.lineNumberWidth + 12
    readonly property int gutterWidth: numberWidth * 2 + (diff.editable ? 22 : 0) + 16
    readonly property real charWidth: metrics.averageCharacterWidth

    // Row of the last line that was clicked, to stage ranges with Shift.
    property int anchorRow: -1

    color: Theme.base
    focus: visible

    Keys.onEscapePressed: detailView.closeFile()

    FontMetrics {
        id: metrics

        font.family: Theme.monoFont
        font.pixelSize: 12
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // File header.
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 44
            color: Theme.panel

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 10
                spacing: 8

                ActionButton {
                    compact: true
                    icon: "close"
                    tip: qsTr("Close the diff (Esc)")
                    onClicked: detailView.closeFile()
                }

                StatusBadge {
                    status: root.diff.status
                }

                Text {
                    Layout.fillWidth: true
                    textFormat: Text.StyledText
                    text: {
                        const path = root.diff.path
                        const slash = path.lastIndexOf("/")
                        const name = "<b>" + path.substring(slash + 1) + "</b>"
                        const dir = slash < 0 ? "" : path.substring(0, slash + 1)
                        const old = root.diff.oldPath !== path && root.diff.oldPath !== ""
                                    ? root.diff.oldPath + " → " : ""
                        return old + dir + name
                    }
                    elide: Text.ElideLeft
                    color: Theme.text
                    font.pixelSize: 13
                }

                Text {
                    visible: root.diff.additions > 0
                    text: "+" + root.diff.additions
                    color: Theme.added
                    font.pixelSize: 12
                    font.bold: true
                }

                Text {
                    visible: root.diff.deletions > 0
                    text: "−" + root.diff.deletions
                    color: Theme.deleted
                    font.pixelSize: 12
                    font.bold: true
                }

                PushButton {
                    visible: root.diff.editable
                    implicitHeight: 26
                    text: qsTr("Discard File")
                    danger: true
                    onClicked: detailView.discardFile(root.diff.path)
                }

                PushButton {
                    visible: root.diff.editable
                    implicitHeight: 26
                    primary: root.diff.stageState !== root.staged
                    text: root.diff.stageState === root.staged ? qsTr("Unstage File")
                                                               : qsTr("Stage File")
                    onClicked: root.diff.setFileStaged(root.diff.stageState !== root.staged)
                }

                ActionButton {
                    compact: true
                    icon: "pencil"
                    hasMenu: true
                    menuOnly: true
                    tip: qsTr("Edit the file")
                    onMenuRequested: (x, y) => root.diff.showEditMenu(-1, x, y)
                }
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: Theme.border
            }
        }

        // Binary files, large diffs and the like.
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: root.diff.notice !== ""

            Column {
                anchors.centerIn: parent
                spacing: 12

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: root.diff.notice
                    color: Theme.textMuted
                    font.pixelSize: 13
                }

                PushButton {
                    anchors.horizontalCenter: parent.horizontalCenter
                    visible: root.diff.canLoadAnyway
                    text: qsTr("Load Anyway")
                    onClicked: root.diff.loadAnyway()
                }
            }
        }

        ListView {
            id: list

            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: root.diff.notice === ""
            clip: true
            model: root.diff
            reuseItems: true
            boundsBehavior: Flickable.StopAtBounds
            flickableDirection: Flickable.AutoFlickIfNeeded
            contentWidth: Math.max(width, root.gutterWidth
                                          + root.diff.maxLineLength * root.charWidth + 40)

            Controls.ScrollBar.vertical: ThinScrollBar {}

            Controls.ScrollBar.horizontal: ThinScrollBar {}

            delegate: Item {
                id: row

                required property int index
                required property int kind
                required property int hunk
                required property string origin
                required property int oldLine
                required property int newLine
                required property string html
                required property bool staged
                required property bool stageable
                required property string header
                required property int hunkState
                required property int resolution
                required property bool chosen

                readonly property bool isHunk: kind === root.hunkRow
                readonly property color background: {
                    switch (origin) {
                    case "+": return Theme.diffAddition
                    case "-": return Theme.diffDeletion
                    case "O": return Theme.diffOurs
                    case "T": return Theme.diffTheirs
                    }
                    return "transparent"
                }

                width: list.contentWidth
                height: isHunk ? 34 : root.lineHeight

                // Hunk header with the hunk actions.
                Rectangle {
                    visible: row.isHunk
                    anchors.fill: parent
                    anchors.topMargin: row.index > 0 ? 6 : 0
                    color: Theme.panel

                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        height: 1
                        color: Theme.border
                    }

                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        width: Math.min(implicitWidth, list.width - hunkActions.width - 40)
                        text: row.header
                        elide: Text.ElideRight
                        color: Theme.textMuted
                        font.family: Theme.monoFont
                        font.pixelSize: 11
                    }

                    Row {
                        id: hunkActions

                        // Stay visible while scrolling horizontally.
                        x: list.contentX + list.width - width - 12
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 6

                        // Conflict resolution.
                        PushButton {
                            visible: root.diff.conflicted
                            implicitHeight: 22
                            text: qsTr("Use Ours")
                            primary: row.resolution === 1
                            onClicked: root.diff.chooseConflict(row.hunk, row.resolution === 1 ? 0 : 1)
                        }

                        PushButton {
                            visible: root.diff.conflicted
                            implicitHeight: 22
                            text: qsTr("Use Theirs")
                            primary: row.resolution === 2
                            onClicked: root.diff.chooseConflict(row.hunk, row.resolution === 2 ? 0 : 2)
                        }

                        PushButton {
                            visible: root.diff.conflicted && row.resolution !== 0
                            implicitHeight: 22
                            text: qsTr("Save")
                            onClicked: root.diff.saveConflict(row.hunk)
                        }

                        PushButton {
                            visible: root.diff.editable && !root.diff.conflicted
                            implicitHeight: 22
                            danger: true
                            text: qsTr("Discard Hunk")
                            onClicked: root.diff.discardHunk(row.hunk)
                        }

                        PushButton {
                            visible: root.diff.editable && !root.diff.conflicted
                            implicitHeight: 22
                            primary: row.hunkState !== root.staged
                            text: row.hunkState === root.staged ? qsTr("Unstage Hunk")
                                                                : qsTr("Stage Hunk")
                            onClicked: root.diff.setHunkStaged(row.hunk,
                                                               row.hunkState !== root.staged)
                        }

                        ActionButton {
                            compact: true
                            icon: "pencil"
                            implicitWidth: 24
                            implicitHeight: 22
                            hasMenu: true
                            menuOnly: true
                            tip: qsTr("Edit the hunk")
                            onMenuRequested: (x, y) => root.diff.showEditMenu(row.hunk, x, y)
                        }
                    }
                }

                // Diff line.
                Rectangle {
                    visible: !row.isHunk
                    anchors.fill: parent
                    color: row.background
                    opacity: row.chosen ? 1 : 0.35

                    Row {
                        anchors.fill: parent

                        Text {
                            width: root.numberWidth
                            height: parent.height
                            rightPadding: 8
                            horizontalAlignment: Text.AlignRight
                            verticalAlignment: Text.AlignVCenter
                            text: row.oldLine > 0 ? row.oldLine : ""
                            color: Theme.textMuted
                            font.family: Theme.monoFont
                            font.pixelSize: 11
                        }

                        Text {
                            width: root.numberWidth
                            height: parent.height
                            rightPadding: 8
                            horizontalAlignment: Text.AlignRight
                            verticalAlignment: Text.AlignVCenter
                            text: row.newLine > 0 ? row.newLine : ""
                            color: Theme.textMuted
                            font.family: Theme.monoFont
                            font.pixelSize: 11
                        }

                        // Stage state of the line.
                        Item {
                            visible: root.diff.editable
                            width: 22
                            height: parent.height

                            Rectangle {
                                visible: row.stageable
                                anchors.centerIn: parent
                                width: 12
                                height: 12
                                radius: 3
                                color: row.staged ? Theme.accent : "transparent"
                                border.color: row.staged ? Theme.accent
                                              : lineMouse.containsMouse ? Theme.text
                                                                        : Theme.textMuted

                                Icon {
                                    visible: row.staged
                                    anchors.centerIn: parent
                                    name: "check"
                                    size: 10
                                    color: Theme.accentText
                                }
                            }

                            MouseArea {
                                id: lineMouse

                                anchors.fill: parent
                                enabled: row.stageable
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: (event) => {
                                    if ((event.modifiers & Qt.ShiftModifier) && root.anchorRow >= 0) {
                                        root.diff.setLinesStaged(Math.min(root.anchorRow, row.index),
                                                                 Math.max(root.anchorRow, row.index),
                                                                 !row.staged)
                                    } else {
                                        root.diff.toggleLine(row.index)
                                    }
                                    root.anchorRow = row.index
                                }
                            }
                        }

                        Text {
                            width: 16
                            height: parent.height
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            text: row.origin === "+" || row.origin === "-" ? row.origin : ""
                            color: row.origin === "+" ? Theme.added : Theme.deleted
                            font.family: Theme.monoFont
                            font.pixelSize: 12
                            font.bold: true
                        }

                        Text {
                            height: parent.height
                            verticalAlignment: Text.AlignVCenter
                            textFormat: Text.RichText
                            text: row.html
                            color: Theme.text
                            font.family: Theme.monoFont
                            font.pixelSize: 12
                            font.strikeout: !row.chosen
                        }
                    }
                }
            }
        }
    }
}
