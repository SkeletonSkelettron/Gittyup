import QtQuick
import QtQuick.Controls.Basic as Controls
import QtQuick.Layouts
import Gittyup

// The main window: the repository tabs, the tool bar, the repository
// sidebar and the page of the current repository, or the welcome page.
// 'mainWindow' is the C++ MainWindow. It adds the pages of the repositories
// to 'pages', each with its own context.
Rectangle {
    id: root

    readonly property int sideBarWidth: 240

    color: Theme.base

    // The tool tip of the item under the mouse, below it, or above it when
    // there's no room below.
    Controls.Popup {
        id: toolTip

        readonly property rect target: host.toolTipRect

        x: Math.max(8, Math.min(target.x, root.width - width - 8))
        y: target.y + target.height + 4 + height <= root.height - 8
           ? target.y + target.height + 4 : target.y - height - 4
        width: Math.min(implicitWidth, 440)
        padding: 6
        leftPadding: 9
        rightPadding: 9
        margins: 8
        focus: false
        closePolicy: Controls.Popup.NoAutoClose
        visible: host.toolTipVisible

        enter: Transition {
            NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 90 }
        }

        contentItem: Text {
            text: host.toolTipText
            textFormat: Text.AutoText
            wrapMode: Text.Wrap
            color: Theme.tooltipText
            font.pixelSize: 12
        }

        background: Rectangle {
            radius: 6
            color: Theme.tooltip
            border.color: Theme.border
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // The menu bar, unless the platform shows the menus.
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 28
            visible: mainWindow.menuBarVisible
            color: Theme.base

            Row {
                anchors.left: parent.left
                anchors.leftMargin: 6
                anchors.verticalCenter: parent.verticalCenter

                Repeater {
                    model: mainWindow.menuTitles

                    delegate: Rectangle {
                        id: title

                        required property int index
                        required property string modelData

                        width: label.implicitWidth + 16
                        height: 24
                        radius: 5
                        color: titleMouse.pressed || titleMouse.containsMouse ? Theme.hover
                                                                              : "transparent"

                        Text {
                            id: label

                            anchors.centerIn: parent
                            text: title.modelData
                            color: Theme.text
                            font.pixelSize: 13
                        }

                        MouseArea {
                            id: titleMouse

                            anchors.fill: parent
                            hoverEnabled: true
                            onPressed: {
                                const p = title.mapToItem(null, 0, title.height + 2)
                                mainWindow.showMenu(title.index, p.x, p.y)
                            }
                        }
                    }
                }
            }
        }

        TabStrip {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
        }

        ToolBar {
            Layout.fillWidth: true
            Layout.preferredHeight: 51
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Theme.border
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // The sidebar slides in and out from the left.
            Item {
                id: sideBarArea

                Layout.fillHeight: true
                Layout.preferredWidth: mainWindow.sideBarVisible ? root.sideBarWidth : 0
                visible: Layout.preferredWidth > 0
                clip: true

                Behavior on Layout.preferredWidth {
                    NumberAnimation { duration: 220; easing.type: Easing.OutCubic }
                }

                FocusScope {
                    anchors.right: parent.right
                    width: root.sideBarWidth
                    height: parent.height

                    SideBar {
                        anchors.fill: parent
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                // The pages of the repositories.
                Item {
                    id: pages

                    objectName: "pages"
                    anchors.fill: parent
                }

                FocusScope {
                    id: welcomeScope

                    anchors.fill: parent
                    visible: mainWindow.welcomeVisible
                    onVisibleChanged: {
                        if (visible)
                            forceActiveFocus()
                    }
                    Component.onCompleted: {
                        if (visible)
                            forceActiveFocus()
                    }

                    WelcomePage {
                        anchors.fill: parent
                    }
                }
            }
        }
    }
}
