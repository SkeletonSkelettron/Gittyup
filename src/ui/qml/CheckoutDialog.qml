import QtQuick
import QtQuick.Layouts
import Gittyup

// 'dialog' is the C++ CheckoutDialog.
DialogPage {
    title: qsTr("Check out")
    subtitle: qsTr("Switch the working directory to a branch, a tag or a remote branch.")
    acceptText: qsTr("Checkout")
    acceptEnabled: dialog.acceptable

    FormField {
        label: qsTr("Reference")

        ComboBox {
            Layout.fillWidth: true
            model: dialog.refs
            textRole: "text"
            currentIndex: dialog.refIndex
            onActivated: (index) => dialog.refIndex = index
            Component.onCompleted: forceActiveFocus()
        }
    }

    CheckBox {
        enabled: dialog.detachEnabled
        text: qsTr("Detach HEAD")
        checked: dialog.detachChecked
        onToggled: dialog.setDetach(checked)
    }
}
