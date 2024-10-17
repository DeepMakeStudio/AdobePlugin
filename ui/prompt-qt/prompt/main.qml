import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Controls.Material

ApplicationWindow {
    width: 300
    height: 180
    visible: true
    title: qsTr("Prompt")
    modality: Qt.WindowModal

    Material.theme: Material.Dark
    Material.accent: Material.Blue

    Rectangle {
        anchors.fill: parent
        color: "transparent"

        TextField {
            anchors.centerIn: parent
            width: parent.width - 20
            placeholderText: "Enter text..."
            text: PromptDialog.promptText

            Component.onCompleted: {
                forceActiveFocus();
                selectAll();
            }

            onTextChanged: {
                PromptDialog.promptText = text
            }
        }

        Row {
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            spacing: 10

            Button {
                text: "Cancel"
                onClicked: Qt.quit()
            }

            Button {
                text: "OK"
                onClicked: {

                    Qt.quit()
                }
            }
        }
    }
}
