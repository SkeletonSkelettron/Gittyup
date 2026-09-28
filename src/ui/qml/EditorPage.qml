import QtQuick
import QtQuick.Controls.Basic as Controls
import QtQuick.Layouts
import Gittyup

// A text editor with the commit that last changed each line on the left.
// 'editor' is the C++ FileEditor.
Rectangle {
    id: root

    readonly property var blame: editor.blame
    readonly property real charWidth: metrics.advanceWidth("0")
    readonly property int numberWidth: charWidth * Math.max(2, String(area.lineCount).length) + 16
    readonly property bool showBlame: blame.hasBlame || blame.blameLoading
    readonly property int blameWidth: showBlame ? 300 : 0
    // All lines of the text area have the same height.
    readonly property real lineHeight: area.lineCount > 0
                                       ? (area.contentHeight - area.topPadding
                                          - area.bottomPadding) / area.lineCount
                                       : metrics.height

    // The commit of the lines under the mouse.
    property string hoveredCommit

    // Scroll the cursor into view, near the top when jumping to a line.
    function showCursor(top) {
        const rect = area.cursorRectangle
        const maxY = Math.max(0, flick.contentHeight - flick.height)
        if (top)
            flick.contentY = Math.max(0, Math.min(rect.y - flick.height / 3, maxY))
        else if (rect.y < flick.contentY)
            flick.contentY = rect.y
        else if (rect.y + rect.height > flick.contentY + flick.height)
            flick.contentY = Math.min(rect.y + rect.height - flick.height, maxY)

        if (rect.x < flick.contentX + 20)
            flick.contentX = Math.max(0, rect.x - 40)
        else if (rect.x > flick.contentX + flick.width - 20)
            flick.contentX = rect.x - flick.width + 80
    }

    color: Theme.base

    FontMetrics {
        id: metrics

        font: area.font
    }

    Connections {
        target: editor

        function onCursorRequested(position) {
            area.cursorPosition = position
            area.forceActiveFocus()
            root.showCursor(true)
        }
    }

    // Select the current match of the find bar.
    Connections {
        target: editor.finder

        function onCurrentChanged(row, start, length) {
            const position = editor.position(row, start)
            area.select(position, position + length)
            root.showCursor(false)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        FindBar {
            Layout.fillWidth: true
            visible: finder.visible
            finder: editor.finder
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            // The gutter follows the text.
            ListView {
                id: gutter

                width: root.blameWidth + root.numberWidth
                height: parent.height
                clip: true
                interactive: false
                model: root.blame
                onCountChanged: contentY = flick.contentY

                delegate: BlameGutter {
                    required number
                    required blameId
                    required blameFirst
                    required blameLast
                    required blameOffset
                    required blameCommitted
                    required blameSummary
                    required blameAuthor
                    required blameDate
                    required blameColor
                    required blameTip

                    height: root.lineHeight
                    blame: root.blame
                    blameWidth: root.blameWidth
                    numberWidth: root.numberWidth
                    hoveredCommit: root.hoveredCommit
                    onHoverRequested: (id) => root.hoveredCommit = id
                }

                // Scroll wheel over the gutter scrolls the text.
                WheelHandler {
                    onWheel: (event) => {
                        const maxY = Math.max(0, flick.contentHeight - flick.height)
                        flick.contentY = Math.max(0, Math.min(flick.contentY - event.angleDelta.y,
                                                              maxY))
                    }
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
                onContentYChanged: gutter.contentY = contentY

                Controls.ScrollBar.vertical: ThinScrollBar {}

                Controls.ScrollBar.horizontal: ThinScrollBar {}

                Controls.TextArea.flickable: Controls.TextArea {
                    id: area

                    textFormat: TextEdit.PlainText
                    wrapMode: TextEdit.NoWrap
                    readOnly: editor.readOnly
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
                    tabStopDistance: editor.tabWidth * root.charWidth
                    background: null
                    focus: true
                    Component.onCompleted: editor.setDocument(textDocument)
                }
            }
        }
    }
}
