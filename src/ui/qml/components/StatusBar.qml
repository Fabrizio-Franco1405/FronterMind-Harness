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

        // App Memory
        Row {
            spacing: 6
            Icon { iconName: "app"; iconColor: "#f38ba8"; iconSize: 12; anchors.verticalCenter: parent.verticalCenter }
            Text { text: "App: " + backend.appMemory; color: "#f38ba8"; font.pixelSize: 12; anchors.verticalCenter: parent.verticalCenter }
        }

        // Model
        Row {
            spacing: 6
            Icon { iconName: "file"; iconColor: "#a6adc8"; iconSize: 12; anchors.verticalCenter: parent.verticalCenter }
            Text { text: backend.currentModel; color: "#a6adc8"; font.pixelSize: 12; anchors.verticalCenter: parent.verticalCenter }
        }

        Item { Layout.fillWidth: true }

        // CPU
        Row {
            spacing: 6
            Icon { iconName: "cpu"; iconColor: "#a6e3a1"; iconSize: 12; anchors.verticalCenter: parent.verticalCenter }
            Text { text: "CPU: " + backend.cpuUsage; color: "#a6e3a1"; font.pixelSize: 12; anchors.verticalCenter: parent.verticalCenter }
        }
        // RAM
        Row {
            spacing: 6
            Icon { iconName: "memory"; iconColor: "#89b4fa"; iconSize: 12; anchors.verticalCenter: parent.verticalCenter }
            Text { text: "RAM: " + backend.ramUsage; color: "#89b4fa"; font.pixelSize: 12; anchors.verticalCenter: parent.verticalCenter }
        }
        // VRAM
        Row {
            spacing: 6
            Icon { iconName: "gpu"; iconColor: "#cba6f7"; iconSize: 12; anchors.verticalCenter: parent.verticalCenter }
            Text { text: "VRAM: " + backend.vramUsage; color: "#cba6f7"; font.pixelSize: 12; anchors.verticalCenter: parent.verticalCenter }
        }
    }
}
