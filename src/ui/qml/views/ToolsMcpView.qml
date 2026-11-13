import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

Item {
    id: root

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        Item {
            width: parent.width
            implicitHeight: mainLayout.implicitHeight + 60

            ColumnLayout {
                id: mainLayout
                width: parent.width - 60
                x: 30
                y: 30
                spacing: 20

                Text {
                    text: "Herramientas del Agente & Servidores MCP"
                    color: theme.textPrimary
                    font.family: theme.fontFamily
                    font.pixelSize: 26
                    font.bold: true
                }

                Text {
                    text: "Estas herramientas se inyectan automáticamente en el protocolo de llama-server (--jinja). El modelo las invoca de forma autónoma cuando lo necesita."
                    color: theme.textSecondary
                    font.family: theme.fontFamily
                    font.pixelSize: 13
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }

                // --- SECCIÓN 1: CONECTAR NUEVO SERVIDOR MCP ---
                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: mcpCol.implicitHeight + 40
                    color: theme.bgSidebar
                    radius: 10
                    border.color: theme.borderColor
                    border.width: 1

                    ColumnLayout {
                        id: mcpCol
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 12

                        Text {
                            text: "Conectar Servidor MCP Externo (JSON-RPC stdio)"
                            color: theme.accentColor
                            font.family: theme.fontFamily
                            font.pixelSize: 15
                            font.bold: true
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12

                            TextField {
                                id: mcpNameInput
                                Layout.preferredWidth: 180
                                Layout.preferredHeight: 36
                                placeholderText: "Alias (ej. GitHub / Fetch)"
                                placeholderTextColor: "#585b70"
                                color: theme.textPrimary
                                background: Rectangle { color: theme.bgBase; border.color: theme.borderColor; radius: 6 }
                            }

                            TextField {
                                id: mcpCmdInput
                                Layout.fillWidth: true
                                Layout.preferredHeight: 36
                                placeholderText: "Comando (ej. npx -y @modelcontextprotocol/server-filesystem /home/fabrizio)"
                                placeholderTextColor: "#585b70"
                                color: theme.textPrimary
                                font.family: theme.monoFontFamily
                                background: Rectangle { color: theme.bgBase; border.color: theme.borderColor; radius: 6 }
                            }

                            Button {
                                text: "Conectar MCP"
                                Layout.preferredWidth: 130
                                Layout.preferredHeight: 36
                                enabled: mcpNameInput.text.trim() !== "" && mcpCmdInput.text.trim() !== ""
                                onClicked: {
                                    if (backend.connectMcpServer(mcpNameInput.text, mcpCmdInput.text)) {
                                        mcpNameInput.text = ""
                                        mcpCmdInput.text = ""
                                    }
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: "#11111b"
                                    font.bold: true
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                background: Rectangle {
                                    color: parent.enabled ? theme.accentColor : "#45475a"
                                    radius: 6
                                }
                            }
                        }
                    }
                }

                // --- SECCIÓN 2: CATÁLOGO DE HERRAMIENTAS ACTIVAS ---
                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: toolsCol.implicitHeight + 40
                    color: theme.bgSidebar
                    radius: 10
                    border.color: theme.borderColor
                    border.width: 1

                    ColumnLayout {
                        id: toolsCol
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 12

                        Text {
                            text: "Herramientas Registradas (" + backend.activeTools.length + " activas)"
                            color: theme.accentColor
                            font.family: theme.fontFamily
                            font.pixelSize: 15
                            font.bold: true
                        }

                        Repeater {
                            model: backend.activeTools

                            delegate: Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 56
                                color: theme.bgBase
                                radius: 8
                                border.color: theme.borderColor
                                border.width: 1

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 14
                                    anchors.rightMargin: 14
                                    spacing: 12

                                    Rectangle {
                                        width: 8; height: 8; radius: 4
                                        color: modelData.isMcp ? "#89b4fa" : "#a6e3a1"
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2

                                        RowLayout {
                                            spacing: 8
                                            Text {
                                                text: modelData.name
                                                color: theme.textPrimary
                                                font.family: theme.monoFontFamily
                                                font.pixelSize: 13
                                                font.bold: true
                                            }
                                            Rectangle {
                                                width: badgeTxt.implicitWidth + 12
                                                height: 18
                                                radius: 4
                                                color: theme.bgSurface
                                                Text {
                                                    id: badgeTxt
                                                    anchors.centerIn: parent
                                                    text: modelData.isMcp ? "MCP SERVER" : "NATIVA C++"
                                                    color: modelData.isMcp ? "#89b4fa" : "#a6e3a1"
                                                    font.pixelSize: 9
                                                    font.bold: true
                                                }
                                            }
                                        }

                                        Text {
                                            text: modelData.description
                                            color: theme.textSecondary
                                            font.pixelSize: 12
                                            elide: Text.ElideRight
                                            Layout.fillWidth: true
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
