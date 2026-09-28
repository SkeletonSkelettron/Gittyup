import QtQuick
import QtQuick.Controls.Basic as Controls
import QtQuick.Layouts
import Gittyup

// The conflicts of a file resolved like in GitKraken: the lines of ours (A)
// and theirs (B) are taken into the output below, which can also be edited.
// 'detailView.merge' is the C++ MergeModel.
Rectangle {
    id: root

    readonly property var merge: detailView.merge
    readonly property real charWidth: metrics.advanceWidth("0")
    readonly property int lineHeight: Math.max(20, Math.ceil(metrics.height) + 4)
    readonly property int numberWidth: charWidth * merge.lineNumberWidth + 12
    readonly property color sideColor0: Theme.diffOurs
    readonly property color sideColor1: Theme.diffTheirs
    readonly property var sideLabels: [merge.oursLabel, merge.theirsLabel]

    // The conflict that the arrows move between.
    property int current: 0

    function sideColor(side) {
        return side === 0 ? root.sideColor0 : root.sideColor1
    }

    function showConflict(index) {
        if (merge.conflictCount === 0)
            return
        current = Math.max(0, Math.min(merge.conflictCount - 1, index))
        oursPane.show(current)
        theirsPane.show(current)
        output.show(current)
    }

    color: Theme.base
    focus: visible

    Keys.onEscapePressed: detailView.closeFile()

    FontMetrics {
        id: metrics

        font.family: Theme.codeFont
        font.pointSize: Theme.codeFontSize
    }

    Connections {
        target: root.merge

        function onLoaded() {
            root.current = 0
        }
    }

    // A letter that marks a side, like in GitKraken.
    component SideBadge: Rectangle {
        property int side

        implicitWidth: 20
        implicitHeight: 20
        radius: 4
        color: Qt.lighter(root.sideColor(side), Theme.dark ? 2.2 : 0.8)

        Text {
            anchors.centerIn: parent
            text: parent.side === 0 ? "A" : "B"
            color: Theme.dark ? Theme.base : "#FFFFFF"
            font.pixelSize: 11
            font.bold: true
        }
    }

    // A check box of a line or of a whole conflict.
    component Check: Rectangle {
        id: check

        // 0 unchecked, 1 partly and 2 checked.
        property int state: 0

        implicitWidth: 14
        implicitHeight: 14
        radius: 3
        color: state > 0 ? Theme.accent : "transparent"
        border.color: state > 0 ? Theme.accent : Theme.textMuted

        Icon {
            anchors.centerIn: parent
            visible: check.state > 0
            name: check.state === 2 ? "check" : "minus"
            size: 10
            color: Theme.accentText
        }
    }

    // The lines of one side with their conflicts.
    component SidePane: Rectangle {
        id: pane

        property int side
        property var model

        function show(conflict) {
            const row = root.merge.conflictRow(pane.side, conflict)
            if (row >= 0)
                list.positionViewAtIndex(row, ListView.Beginning)
        }

        color: Theme.base

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 32
                color: Theme.panel

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    spacing: 8

                    SideBadge {
                        side: pane.side
                    }

                    Text {
                        Layout.fillWidth: true
                        text: root.sideLabels[pane.side]
                        elide: Text.ElideRight
                        color: Theme.text
                        font.pixelSize: 12
                        font.bold: true
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

            ListView {
                id: list

                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: pane.model
                reuseItems: true
                boundsBehavior: Flickable.StopAtBounds
                flickableDirection: Flickable.AutoFlickIfNeeded
                contentWidth: Math.max(width, 30 + root.numberWidth + 2000)

                Controls.ScrollBar.vertical: ThinScrollBar {}

                Controls.ScrollBar.horizontal: ThinScrollBar {}

                delegate: Rectangle {
                    id: row

                    required property int kind
                    required property int conflict
                    required property int line
                    required property int number
                    required property string html
                    required property bool checked
                    required property int checkState

                    readonly property bool isConflict: kind === 1
                    readonly property bool isLine: kind === 2

                    width: list.contentWidth
                    height: isConflict ? 26 : root.lineHeight
                    color: isConflict ? Theme.panel
                           : isLine ? Qt.rgba(root.sideColor(pane.side).r,
                                              root.sideColor(pane.side).g,
                                              root.sideColor(pane.side).b,
                                              checked ? 1 : 0.45)
                           : "transparent"

                    // The header of a conflict takes all its lines.
                    RowLayout {
                        visible: row.isConflict
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        spacing: 8

                        Check {
                            state: row.checkState
                        }

                        Text {
                            text: qsTr("Conflict %1").arg(row.conflict + 1)
                            color: row.conflict === root.current ? Theme.accent : Theme.textMuted
                            font.pixelSize: 11
                            font.bold: true
                        }

                        Item { Layout.fillWidth: true }
                    }

                    Row {
                        visible: !row.isConflict
                        anchors.fill: parent

                        Item {
                            width: 30
                            height: parent.height

                            Check {
                                visible: row.isLine
                                anchors.centerIn: parent
                                state: row.checked ? 2 : 0
                            }
                        }

                        Text {
                            width: root.numberWidth
                            height: parent.height
                            rightPadding: 8
                            horizontalAlignment: Text.AlignRight
                            verticalAlignment: Text.AlignVCenter
                            text: row.number
                            color: Theme.textMuted
                            font.family: Theme.codeFont
                            font.pointSize: Math.max(7, Theme.codeFontSize - 1)
                        }

                        Text {
                            height: parent.height
                            verticalAlignment: Text.AlignVCenter
                            textFormat: Text.RichText
                            text: row.html
                            color: Theme.text
                            font.family: Theme.codeFont
                            font.pointSize: Theme.codeFontSize
                        }
                    }

                    // Clicking a line of a conflict or its header takes it.
                    MouseArea {
                        anchors.fill: parent
                        enabled: row.isConflict || row.isLine
                        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                        onClicked: {
                            root.current = row.conflict
                            if (row.isConflict)
                                root.merge.setConflictChecked(pane.side, row.conflict,
                                                              row.checkState !== 2)
                            else
                                root.merge.setLineChecked(pane.side, row.conflict,
                                                          row.line, !row.checked)
                        }
                    }
                }
            }
        }
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
                    tip: qsTr("Close the merge editor (Esc)")
                    onClicked: detailView.closeFile()
                }

                StatusBadge {
                    status: "!"
                }

                Text {
                    Layout.fillWidth: true
                    textFormat: Text.StyledText
                    text: {
                        const path = root.merge.path
                        const slash = path.lastIndexOf("/")
                        return (slash < 0 ? "" : path.substring(0, slash + 1))
                               + "<b>" + path.substring(slash + 1) + "</b>"
                    }
                    elide: Text.ElideLeft
                    color: Theme.text
                    font.pixelSize: 13
                }

                ActionButton {
                    compact: true
                    implicitWidth: 28
                    implicitHeight: 28
                    visible: root.merge.conflictCount > 0
                    enabled: root.current > 0
                    icon: "chevron-up"
                    tip: qsTr("Previous conflict")
                    onClicked: root.showConflict(root.current - 1)
                }

                Text {
                    visible: root.merge.conflictCount > 0
                    text: qsTr("Conflict %1 of %2").arg(root.current + 1)
                                                    .arg(root.merge.conflictCount)
                    color: Theme.textMuted
                    font.pixelSize: 12
                }

                ActionButton {
                    compact: true
                    implicitWidth: 28
                    implicitHeight: 28
                    visible: root.merge.conflictCount > 0
                    enabled: root.current < root.merge.conflictCount - 1
                    icon: "chevron-down"
                    tip: qsTr("Next conflict")
                    onClicked: root.showConflict(root.current + 1)
                }

                PushButton {
                    implicitHeight: 26
                    visible: root.merge.conflictCount > 0
                    text: qsTr("Take All A")
                    tip: qsTr("Take all lines of %1").arg(root.merge.oursLabel)
                    onClicked: root.merge.takeAll(0)
                }

                PushButton {
                    implicitHeight: 26
                    visible: root.merge.conflictCount > 0
                    text: qsTr("Take All B")
                    tip: qsTr("Take all lines of %1").arg(root.merge.theirsLabel)
                    onClicked: root.merge.takeAll(1)
                }

                MergeModeSwitch {}

                PushButton {
                    implicitHeight: 26
                    primary: true
                    enabled: root.merge.notice === ""
                    text: qsTr("Save")
                    tip: qsTr("Save the output and mark the conflicts as resolved")
                    onClicked: root.merge.save()
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

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: root.merge.notice !== ""

            Text {
                anchors.centerIn: parent
                text: root.merge.notice
                color: Theme.textMuted
                font.pixelSize: 13
            }
        }

        Controls.SplitView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: root.merge.notice === ""
            orientation: Qt.Vertical

            handle: Rectangle {
                implicitHeight: 5
                color: Controls.SplitHandle.hovered || Controls.SplitHandle.pressed
                       ? Theme.accent : Theme.border
            }

            Controls.SplitView {
                Controls.SplitView.preferredHeight: parent.height * 0.5
                Controls.SplitView.minimumHeight: 120
                orientation: Qt.Horizontal

                handle: Rectangle {
                    implicitWidth: 5
                    color: Controls.SplitHandle.hovered || Controls.SplitHandle.pressed
                           ? Theme.accent : Theme.border
                }

                SidePane {
                    id: oursPane

                    Controls.SplitView.preferredWidth: parent.width / 2
                    Controls.SplitView.minimumWidth: 160
                    side: 0
                    model: root.merge.ours
                }

                SidePane {
                    id: theirsPane

                    Controls.SplitView.fillWidth: true
                    Controls.SplitView.minimumWidth: 160
                    side: 1
                    model: root.merge.theirs
                }
            }

            MergeOutput {
                id: output

                Controls.SplitView.fillHeight: true
                Controls.SplitView.minimumHeight: 120
                merge: root.merge
                lineHeight: root.lineHeight
                numberWidth: root.numberWidth
                sideColors: [root.sideColor0, root.sideColor1]
            }
        }
    }
}
