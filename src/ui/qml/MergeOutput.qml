import QtQuick
import QtQuick.Controls.Basic as Controls
import QtQuick.Layouts
import Gittyup

// The output of the merge editor: the common lines and the lines taken from
// each side, colored like the side they come from. It can be edited.
// 'merge' is the C++ MergeModel.
Rectangle {
    id: root

    property var merge
    property int lineHeight
    property int numberWidth
    property var sideColors: []

    // The top of a position of the text.
    function positionY(position) {
        // Follow the layout of the text.
        area.contentHeight
        area.width
        return area.positionToRectangle(position).y
    }

    function rowY(row) {
        area.contentHeight
        area.width
        if (row > root.merge.outputRow(area.length))
            return area.contentHeight
        return area.positionToRectangle(root.merge.outputPosition(row)).y
    }

    // Scroll to the lines of a conflict.
    function show(conflict) {
        const y = positionY(root.merge.regionStart(conflict))
        const maxY = Math.max(0, flick.contentHeight - flick.height)
        flick.contentY = Math.max(0, Math.min(y - 40, maxY))
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

                Text {
                    text: qsTr("Output")
                    color: Theme.text
                    font.pixelSize: 12
                    font.bold: true
                }

                Item { Layout.fillWidth: true }

                Text {
                    text: root.merge.unresolvedCount === 0
                          ? qsTr("All conflicts have a resolution")
                          : root.merge.unresolvedCount === 1
                            ? qsTr("1 conflict to resolve")
                            : qsTr("%1 conflicts to resolve").arg(root.merge.unresolvedCount)
                    color: root.merge.unresolvedCount === 0 ? Theme.added : Theme.modified
                    font.pixelSize: 12
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

            // The numbers of the lines on the screen.
            Item {
                id: gutter

                readonly property int firstRow: {
                    area.contentHeight
                    return root.merge.outputRow(area.positionAt(0, flick.contentY))
                }
                readonly property int lastRow: {
                    area.contentHeight
                    return root.merge.outputRow(area.positionAt(0, flick.contentY + flick.height))
                }

                width: root.numberWidth + 30
                height: parent.height
                clip: true

                Repeater {
                    model: Math.max(0, gutter.lastRow - gutter.firstRow + 1)

                    delegate: Text {
                        required property int index
                        readonly property int row: gutter.firstRow + index

                        y: root.rowY(row) - flick.contentY
                        width: gutter.width - 1
                        height: area.cursorRectangle.height
                        rightPadding: 8
                        horizontalAlignment: Text.AlignRight
                        verticalAlignment: Text.AlignVCenter
                        text: row + 1
                        color: Theme.textMuted
                        font.family: Theme.codeFont
                        font.pointSize: Math.max(7, Theme.codeFontSize - 1)
                    }
                }

                Rectangle {
                    anchors.right: parent.right
                    width: 1
                    height: parent.height
                    color: Theme.border
                }
            }

            Flickable {
                id: flick

                anchors.left: gutter.right
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                clip: true
                boundsBehavior: Flickable.StopAtBounds

                Controls.ScrollBar.vertical: ThinScrollBar {}

                Controls.ScrollBar.horizontal: ThinScrollBar {}

                Controls.TextArea.flickable: Controls.TextArea {
                    id: area

                    textFormat: TextEdit.PlainText
                    wrapMode: TextEdit.NoWrap
                    selectByMouse: true
                    persistentSelection: true
                    leftPadding: 8
                    rightPadding: 8
                    topPadding: 0
                    bottomPadding: 0
                    color: Theme.text
                    selectionColor: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.35)
                    selectedTextColor: Theme.text
                    font.family: Theme.codeFont
                    font.pointSize: Theme.codeFontSize
                    tabStopDistance: 4 * metrics.advanceWidth("0")
                    background: null
                    Component.onCompleted: root.merge.setDocument(textDocument)

                    FontMetrics {
                        id: metrics

                        font: area.font
                    }

                    // The lines of the conflicts, behind the text.
                    Repeater {
                        model: root.merge.regions

                        delegate: Item {
                            id: region

                            required property int index
                            required property var modelData

                            readonly property real startY: root.positionY(modelData.start)
                            readonly property real middleY: root.positionY(modelData.middle)
                            readonly property real endY: root.positionY(modelData.end)
                            readonly property bool empty: modelData.end === modelData.start

                            z: -1
                            width: area.width
                            height: area.height

                            Rectangle {
                                visible: region.middleY > region.startY
                                y: region.startY
                                width: parent.width
                                height: region.middleY - region.startY
                                color: root.sideColors[region.modelData.first]
                            }

                            Rectangle {
                                visible: region.endY > region.middleY
                                y: region.middleY
                                width: parent.width
                                height: region.endY - region.middleY
                                color: root.sideColors[1 - region.modelData.first]
                            }

                            // A conflict without lines is a line between the others.
                            Rectangle {
                                visible: region.empty
                                y: region.startY - 1
                                width: parent.width
                                height: 2
                                color: region.modelData.touched ? Theme.textMuted : Theme.modified
                            }

                            Rectangle {
                                visible: region.empty
                                x: parent.width - width - 12
                                y: region.startY - height / 2
                                implicitWidth: markerText.implicitWidth + 12
                                implicitHeight: 16
                                radius: 8
                                color: region.modelData.touched ? Theme.textMuted : Theme.modified

                                Text {
                                    id: markerText

                                    anchors.centerIn: parent
                                    text: region.modelData.touched
                                          ? qsTr("Conflict %1: no lines").arg(region.index + 1)
                                          : qsTr("Conflict %1").arg(region.index + 1)
                                    color: Theme.base
                                    font.pixelSize: 10
                                    font.bold: true
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
