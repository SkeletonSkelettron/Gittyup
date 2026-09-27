import QtQuick
import QtQuick.Controls.Basic as Controls
import Gittyup

// The repository page: references on the left and the commit graph.
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
            Controls.SplitView.preferredWidth: 240
            Controls.SplitView.minimumWidth: 160
            Controls.SplitView.maximumWidth: 480
        }

        GraphView {
            Controls.SplitView.fillWidth: true
            Controls.SplitView.minimumWidth: 300
        }
    }
}
