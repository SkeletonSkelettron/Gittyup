import QtQuick
import QtQuick.Controls.Basic as Controls
import Gittyup

// The repository page: references on the left, the commit graph or the diff
// of the selected file in the middle and the details on the right.
Rectangle {
    id: root

    color: Theme.base

    Controls.SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        handle: Item {
            implicitWidth: 5

            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                width: 1
                height: parent.height
                color: Controls.SplitHandle.hovered || Controls.SplitHandle.pressed
                       ? Theme.accent : "transparent"
            }
        }

        RefsPanel {
            visible: !repoView.maximized
            Controls.SplitView.preferredWidth: 240
            Controls.SplitView.minimumWidth: 160
            Controls.SplitView.maximumWidth: 480
        }

        Item {
            Controls.SplitView.fillWidth: true
            Controls.SplitView.minimumWidth: 300

            GraphView {
                anchors.fill: parent
                visible: detailView.selectedFile === ""
            }

            DiffPanel {
                anchors.fill: parent
                visible: detailView.selectedFile !== ""
            }
        }

        DetailsPanel {
            visible: !repoView.maximized
            Controls.SplitView.preferredWidth: 360
            Controls.SplitView.minimumWidth: 280
            Controls.SplitView.maximumWidth: 640
        }
    }
}
