import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root
    height: 30
    color: "#11111b" // Muy oscuro

    // Línea superior sutil
    Rectangle {
        width: parent.width
        height: 1
        color: "#313244"
        anchors.top: parent.top
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 15
        anchors.rightMargin: 15
        spacing: 20

        // Memoria de la App (Alineado a la izquierda)
        Text { text: "App: " + backend.appMemory; color: "#f38ba8"; font.pixelSize: 12 }

        Text { text: "Modelo: " + backend.currentModel; color: "#a6adc8"; font.pixelSize: 12 }

        Item { Layout.fillWidth: true } // <-- Este espaciador empuja el resto a la derecha

        Text { text: "CPU: " + backend.cpuUsage; color: "#a6e3a1"; font.pixelSize: 12 }
        Text { text: "RAM: " + backend.ramUsage; color: "#89b4fa"; font.pixelSize: 12 }
        Text { text: "VRAM: " + backend.vramUsage; color: "#cba6f7"; font.pixelSize: 12 }
    }
}
