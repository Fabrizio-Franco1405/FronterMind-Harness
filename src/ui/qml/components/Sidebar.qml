import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: sidebar
    width: 235
    color: theme.bgSidebar

    signal viewRequested(string viewName)
    property int currentIndex: 0

    Rectangle {
        width: 1
        height: parent.height
        color: theme.borderColor
        anchors.right: parent.right
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            Layout.bottomMargin: 18
            spacing: 10

            Rectangle {
                width: 36; height: 36; radius: 9
                color: "#11111b"
                border.color: theme.accentColor
                border.width: 1
                clip: true

                Image {
                    anchors.fill: parent
                    anchors.margins: 3
                    sourceSize: Qt.size(64, 64)
                    source: "qrc:/assets/logo.svg"
                    fillMode: Image.PreserveAspectCrop
                    smooth: true
                }
            }

            Column {
                spacing: 1
                Text {
                    text: "FronterMind"
                    color: theme.textPrimary
                    font.family: theme.fontFamily
                    font.pixelSize: 17
                    font.bold: true
                }
                Text {
                    text: "LOCAL AI HARNESS"
                    color: theme.textSecondary
                    font.family: theme.fontFamily
                    font.pixelSize: 9
                    font.bold: true
                    font.letterSpacing: 1.1
                }
            }
        }

        Text {
            text: "ESPACIO DE TRABAJO"
            color: theme.textSecondary
            font.family: theme.fontFamily
            font.pixelSize: 10
            font.bold: true
            font.letterSpacing: 0.8
            Layout.leftMargin: 6
            Layout.bottomMargin: 4
        }

        NavButton {
            text: "Agente & Chat"
            iconCode: "comment"
            isActive: sidebar.currentIndex === 0
            onClicked: sidebar.viewRequested("chat")
        }

        NavButton {
            text: "Motor & Modelos"
            iconCode: "settings"
            isActive: sidebar.currentIndex === 1
            onClicked: sidebar.viewRequested("settings")
        }

        NavButton {
            text: "Herramientas & MCP"
            iconCode: "build"
            isActive: sidebar.currentIndex === 2
            onClicked: sidebar.viewRequested("tools")
        }

        NavButton {
            text: "Apariencia & Temas"
            iconCode: "paint-palette"
            isActive: sidebar.currentIndex === 3
            onClicked: sidebar.viewRequested("appearance")
        }

        Item { Layout.fillHeight: true }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 54
            radius: 8
            color: theme.bgDarkest
            border.color: theme.borderColor
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                Rectangle {
                    width: 8; height: 8; radius: 4
                    color: backend.engineManager.isRunning ? "#a6e3a1" : "#f38ba8"
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 1
                    Text {
                        text: backend.engineManager.isRunning ? "Servidor Activo" : "Servidor Detenido"
                        color: theme.textPrimary
                        font.family: theme.fontFamily
                        font.pixelSize: 11
                        font.bold: true
                    }
                    Text {
                        text: backend.currentEngine + " • 127.0.0.1:8080"
                        color: theme.textSecondary
                        font.family: theme.fontFamily
                        font.pixelSize: 10
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 4
            Text {
                text: "v0.1.0 Alpha"
                color: theme.textSecondary
                font.family: theme.fontFamily
                font.pixelSize: 11
            }
            Item { Layout.fillWidth: true }
        }
    }
}
