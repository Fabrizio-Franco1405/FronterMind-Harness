import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        // Historial de mensajes
        ListView {
            id: messageList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 10
            model: backend.chatMessages // Enlazado al QStringList de C++

            delegate: Rectangle {
                width: messageList.width
                height: messageText.implicitHeight + 20
                color: "transparent"

                Text {
                    id: messageText
                    text: modelData
                    color: modelData.startsWith("Tú:") ? "#89b4fa" : "#cdd6f4"
                    font.pixelSize: 14
                    wrapMode: Text.WordWrap
                    width: parent.width
                }
            }
        }

        // Barra de entrada
        ChatInput {
            Layout.fillWidth: true
        }
    }
}
