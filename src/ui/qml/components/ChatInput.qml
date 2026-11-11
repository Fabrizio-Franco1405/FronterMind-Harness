import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    height: 60
    color: "#1e1e2e"
    radius: 8
    border.color: "#313244"
    border.width: 1

    property string placeholderText: "Escribe un mensaje..."

    TextField {
        id: input
        anchors.fill: parent
        anchors.margins: 10
        anchors.rightMargin: 60 // Espacio para el botón
        placeholderText: root.placeholderText
        color: "#cdd6f4"
        font.pixelSize: 14
        background: null // Quitamos el fondo por defecto para usar el del Rectangle

        // Enviar con Enter
        onAccepted: {
            if (text.trim() !== "") {
                backend.sendMessage(text)
                text = ""
            }
        }
    }

    Button {
        text: "➤"
        width: 40
        height: 40
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        background: Rectangle {
            color: parent.hovered ? "#cba6f7" : "#45475a"
            radius: 6
        }
        contentItem: Text {
            text: parent.text
            color: "#1e1e2e"
            font.pixelSize: 16
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        onClicked: {
            if (input.text.trim() !== "") {
                backend.sendMessage(input.text)
                input.text = ""
            }
        }
    }
}
