import QtQuick
import QtQuick.Layouts
import "../components"

Rectangle {
    id: root
    height: 32
    color: "#11111b"

    Rectangle { width: parent.width; height: 1; color: "#313244"; anchors.top: parent.top }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 15
        anchors.rightMargin: 15
        spacing: 20

        // App Memory (Icono de Terminal)
        Row {
            spacing: 6
            Icon { iconCode: "\uf120"; iconColor: "#f38ba8"; font.pixelSize: 12; anchors.verticalCenter: parent.verticalCenter }
            Text { text: "App: " + backend.appMemory; color: "#f38ba8"; font.pixelSize: 12; anchors.verticalCenter: parent.verticalCenter }
        }

        // Model (Icono de Carpeta)
        Row {
            spacing: 6
            Icon { iconCode: "\uf07b"; iconColor: "#a6adc8"; font.pixelSize: 12; anchors.verticalCenter: parent.verticalCenter }
            Text { text: backend.currentModel; color: "#a6adc8"; font.pixelSize: 12; anchors.verticalCenter: parent.verticalCenter }
        }

        Item { Layout.fillWidth: true }

        // CPU (Icono de Microchip)
        Row {
            spacing: 6
            Icon { iconCode: "\uf2db"; iconColor: "#a6e3a1"; font.pixelSize: 12; anchors.verticalCenter: parent.verticalCenter }
            Text { text: "CPU: " + backend.cpuUsage; color: "#a6e3a1"; font.pixelSize: 12; anchors.verticalCenter: parent.verticalCenter }
        }
        // RAM (Icono de Servidor/Memoria)
        Row {
            spacing: 6
            Icon { iconCode: "\uf538"; iconColor: "#89b4fa"; font.pixelSize: 12; anchors.verticalCenter: parent.verticalCenter }
            Text { text: "RAM: " + backend.ramUsage; color: "#89b4fa"; font.pixelSize: 12; anchors.verticalCenter: parent.verticalCenter }
        }
        // VRAM (Icono de Escritorio/GPU)
        Row {
            spacing: 6
            Icon { iconCode: "\uf108"; iconColor: "#cba6f7"; font.pixelSize: 12; anchors.verticalCenter: parent.verticalCenter }
            Text { text: "VRAM: " + backend.vramUsage; color: "#cba6f7"; font.pixelSize: 12; anchors.verticalCenter: parent.verticalCenter }
        }
    }
}
