import QtQuick
import QtQuick.Controls

ApplicationWindow {
    width: 800
    height: 600
    visible: true
    title: "FronterMind - Harness"
    color: "#1e1e2e"

    Column {
        anchors.centerIn: parent
        spacing: 20

        Text {
            text: backend.statusText
            color: "#cdd6f4"
            font.pixelSize: 24
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Button {
            text: "Probar comunicación con el Core"
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                backend.handleUserAction("¡Hola desde QML!")
            }
        }
    }
}
