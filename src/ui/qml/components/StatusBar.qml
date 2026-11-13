import QtQuick
import QtQuick.Layouts
import "../components"

Rectangle {
    id: root
    height: 34
    color: "#11111b"

    Rectangle { width: parent.width; height: 1; color: "#313244"; anchors.top: parent.top }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        spacing: 14

        // App Memory Pill
        Rectangle {
            height: 22
            width: appRow.implicitWidth + 16
            radius: 4
            color: "#181825"
            border.color: "#313244"
            Row {
                id: appRow
                anchors.centerIn: parent
                spacing: 6
                Icon { iconName: "app"; iconColor: "#f38ba8"; iconSize: 11; anchors.verticalCenter: parent.verticalCenter }
                Text {
                    text: "App: " + backend.appMemory
                    color: "#cdd6f4"
                    font.pixelSize: 11
                    font.family: "monospace"
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }

        // Modelo activo (truncado elegante para no empujar la barra)
        RowLayout {
            spacing: 6
            Layout.fillWidth: true
            Icon { iconName: "file"; iconColor: "#cba6f7"; iconSize: 12 }
            Text {
                text: backend.currentModel
                color: "#a6adc8"
                font.pixelSize: 11
                elide: Text.ElideMiddle
                Layout.fillWidth: true
            }
        }

        // CPU
        Row {
            spacing: 5
            Icon { iconName: "cpu"; iconColor: "#a6e3a1"; iconSize: 12; anchors.verticalCenter: parent.verticalCenter }
            Text { text: "CPU:"; color: "#6c7086"; font.pixelSize: 11; anchors.verticalCenter: parent.verticalCenter }
            Text { text: backend.cpuUsage; color: "#a6e3a1"; font.pixelSize: 11; font.family: "monospace"; font.bold: true; anchors.verticalCenter: parent.verticalCenter }
        }

        Rectangle { width: 1; height: 12; color: "#313244" }

        // RAM
        Row {
            spacing: 5
            Icon { iconName: "memory"; iconColor: "#89b4fa"; iconSize: 12; anchors.verticalCenter: parent.verticalCenter }
            Text { text: "RAM:"; color: "#6c7086"; font.pixelSize: 11; anchors.verticalCenter: parent.verticalCenter }
            Text { text: backend.ramUsage; color: "#89b4fa"; font.pixelSize: 11; font.family: "monospace"; font.bold: true; anchors.verticalCenter: parent.verticalCenter }
        }

        Rectangle { width: 1; height: 12; color: "#313244" }

        // VRAM
        Row {
            spacing: 5
            Icon { iconName: "gpu"; iconColor: "#cba6f7"; iconSize: 12; anchors.verticalCenter: parent.verticalCenter }
            Text { text: "VRAM:"; color: "#6c7086"; font.pixelSize: 11; anchors.verticalCenter: parent.verticalCenter }
            Text { text: backend.vramUsage; color: "#cba6f7"; font.pixelSize: 11; font.family: "monospace"; font.bold: true; anchors.verticalCenter: parent.verticalCenter }
        }
    }
}
