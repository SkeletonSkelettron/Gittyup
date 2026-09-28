import QtQuick
import QtQuick.Layouts
import Gittyup

// 'dialog' is the C++ AddRemoteDialog.
DialogPage {
    title: qsTr("Add a remote")
    acceptText: qsTr("Add Remote")
    acceptEnabled: dialog.acceptable

    FormField {
        label: qsTr("Name")

        TextField {
            Layout.fillWidth: true
            placeholderText: qsTr("origin")
            text: dialog.name
            onTextEdited: dialog.name = text
            Component.onCompleted: if (text === "") forceActiveFocus()
        }
    }

    FormField {
        label: qsTr("URL")

        TextField {
            Layout.fillWidth: true
            placeholderText: qsTr("https://example.com/user/repository.git")
            text: dialog.url
            onTextEdited: dialog.url = text
            Component.onCompleted: if (dialog.name !== "") forceActiveFocus()
        }
    }
}
