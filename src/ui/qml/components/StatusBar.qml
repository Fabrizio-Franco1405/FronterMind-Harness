import QtQuick
import QtQuick.Layouts
import "../components"

Rectangle {
    id: root
    height: 34
    color: theme.bgDarkest

    Rectangle { width: parent.width; height: 1; color: theme.borderColor; anchors.top: parent.top }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        spacing: 14

        // Izquierda: Cápsula de Memoria de la App
        Rectangle {
            Layout.preferredHeight: 22
            Layout.preferredWidth: appText.implicitWidth + 24
            radius: 5
            color: theme.bgSidebar
            border.color: theme.borderColor
            border.width: 1

            Row {
                anchors.centerIn: parent
                spacing: 6

                Rectangle {
                    width: 6; height: 6; radius: 3
                    color: "#f38ba8"
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    id: appText
                    text: "App: " + backend.appMemory
                    color: theme.textPrimary
                    font.pixelSize: 11
                    font.family: theme.monoFontFamily
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }

        // Espaciador elástico: empuja CPU, RAM y VRAM al extremo derecho
        Item {
            Layout.fillWidth: true
        }

        // Derecha: CPU
        Row {
            spacing: 5
            Text { text: "CPU:"; color: theme.textSecondary; font.family: theme.fontFamily; font.pixelSize: 11; anchors.verticalCenter: parent.verticalCenter }
            Text { text: backend.cpuUsage; color: "#a6e3a1"; font.pixelSize: 11; font.family: theme.monoFontFamily; font.bold: true; anchors.verticalCenter: parent.verticalCenter }
        }

        Rectangle { width: 1; height: 12; color: theme.borderColor }

        // Derecha: RAM
        Row {
            spacing: 5
            Text { text: "RAM:"; color: theme.textSecondary; font.family: theme.fontFamily; font.pixelSize: 11; anchors.verticalCenter: parent.verticalCenter }
            Text { text: backend.ramUsage; color: "#89b4fa"; font.pixelSize: 11; font.family: theme.monoFontFamily; font.bold: true; anchors.verticalCenter: parent.verticalCenter }
        }

        Rectangle { width: 1; height: 12; color: theme.borderColor }

        // Derecha: VRAM
        Row {
            spacing: 5
            Text { text: "VRAM:"; color: theme.textSecondary; font.family: theme.fontFamily; font.pixelSize: 11; anchors.verticalCenter: parent.verticalCenter }
            Text { text: backend.vramUsage; color: theme.accentColor; font.pixelSize: 11; font.family: theme.monoFontFamily; font.bold: true; anchors.verticalCenter: parent.verticalCenter }
        }
    }
}
