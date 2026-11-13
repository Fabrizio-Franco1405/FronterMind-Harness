import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"
import FronterMind.Components.MarkdownViewer

Item {
    id: root

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // --- HEADER SUPERIOR ---
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 72
            color: "#1e1e2e"

            Rectangle { width: parent.width; height: 1; color: "#313244"; anchors.bottom: parent.bottom }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 30
                anchors.rightMargin: 30
                spacing: 18

                ColumnLayout {
                    spacing: 4
                    Text {
                        text: "Agente Asistente"
                        color: "#cdd6f4"
                        font.pixelSize: 18
                        font.bold: true
                    }

                    RowLayout {
                        spacing: 8
                        Icon { iconName: "database"; iconColor: "#cba6f7"; iconSize: 11 }
                        Text {
                            text: backend.usedTokens + " / " + backend.maxContext + " tokens"
                            color: "#a6adc8"
                            font.pixelSize: 11
                            font.family: "monospace"
                        }
                        Rectangle {
                            width: 70; height: 5; radius: 3
                            color: "#313244"
                            Rectangle {
                                width: Math.min(parent.width, (backend.usedTokens / Math.max(1, backend.maxContext)) * parent.width)
                                height: parent.height
                                radius: 3
                                color: (backend.usedTokens / Math.max(1, backend.maxContext)) > 0.85 ? "#f38ba8" : "#cba6f7"
                                Behavior on width { NumberAnimation { duration: 180 } }
                            }
                        }
                    }
                }

                Item { Layout.fillWidth: true }

                Text { text: "Modelo:"; color: "#a6adc8"; font.pixelSize: 13 }

                ComboBox {
                    id: modelSelector
                    Layout.preferredWidth: 250
                    Layout.preferredHeight: 36
                    model: backend.localModels
                    textRole: "alias"
                    onActivated: {
                        var m = backend.localModels[currentIndex]
                        backend.loadModel(m.alias, m.path)
                    }
                    contentItem: Text {
                        leftPadding: 12
                        rightPadding: 24
                        text: modelSelector.currentIndex >= 0 ? modelSelector.textAt(modelSelector.currentIndex) : "Seleccionar modelo"
                        color: "#cdd6f4"
                        font.pixelSize: 13
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: "#181825"
                        border.color: modelSelector.hovered ? "#cba6f7" : "#45475a"
                        radius: 8
                        Behavior on border.color { ColorAnimation { duration: 100 } }
                    }
                }

                // Botón Limpiar Sesión
                Rectangle {
                    Layout.preferredWidth: 36
                    Layout.preferredHeight: 36
                    radius: 8
                    color: clearMouse.containsMouse ? "#313244" : "#181825"
                    border.color: clearMouse.containsMouse ? "#f38ba8" : "#45475a"
                    border.width: 1
                    Behavior on color { ColorAnimation { duration: 100 } }

                    Icon {
                        anchors.centerIn: parent
                        iconName: "trash"
                        iconColor: "#f38ba8"
                        iconSize: 15
                    }

                    MouseArea {
                        id: clearMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: backend.clearChat()
                    }
                }
            }
        }

        // --- ÁREA DE MENSAJES ---
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 14
                visible: backend.chatMessages.length === 0

                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    width: 72; height: 72; radius: 20
                    color: "#181825"
                    border.color: "#45475a"
                    border.width: 1

                    Icon {
                        anchors.centerIn: parent
                        iconName: "stars"
                        iconColor: "#cba6f7"
                        iconSize: 36
                    }
                }

                Text {
                    text: "¿En qué estamos trabajando hoy?"
                    color: "#cdd6f4"
                    font.pixelSize: 24
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: "Tu agente local con memoria contextual está listo para analizar código, diseñar arquitectura o depurar errores."
                    color: "#6c7086"
                    font.pixelSize: 13
                    Layout.alignment: Qt.AlignHCenter
                }
            }

            ListView {
                id: messageList
                anchors.fill: parent
                clip: true
                spacing: 22
                cacheBuffer: 2000 // Mantiene delegados en caché para scroll instantáneo
                model: backend.chatMessages
                visible: backend.chatMessages.length > 0

                onCountChanged: Qt.callLater(positionViewAtEnd)
                topMargin: 28; bottomMargin: 28; leftMargin: 30; rightMargin: 30

                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AsNeeded
                }

                delegate: Item {
                    width: messageList.width - 60
                    height: bubble.height

                    property bool isUser: modelData.startsWith("Tú:")
                    property bool isThinking: modelData === "Agente: Pensando..." || modelData.startsWith("Agente: Pensando")
                    property bool isSystem: modelData.startsWith("Sistema:")
                    property bool isMarkdown: !isUser && !isSystem && !isThinking

                    Rectangle {
                        id: avatar
                        width: 34; height: 34; radius: 10
                        color: isUser ? "#89b4fa" : "#181825"
                        border.color: isUser ? "transparent" : "#cba6f7"
                        border.width: isUser ? 0 : 1
                        anchors.left: isUser ? undefined : parent.left
                        anchors.right: isUser ? parent.right : undefined
                        anchors.top: parent.top

                        Icon {
                            anchors.centerIn: parent
                            iconName: isUser ? "user" : "stars"
                            iconColor: isUser ? "#11111b" : "#cba6f7"
                            iconSize: 16
                        }
                    }

                    Rectangle {
                        id: bubble
                        width: (isSystem || isThinking)
                               ? statusRow.implicitWidth + 32
                               : (isUser ? Math.min(parent.width * 0.78, userText.implicitWidth + 28) : parent.width * 0.86)
                        height: (isSystem || isThinking)
                                ? Math.max(40, statusText.implicitHeight + 24)
                                : (isUser ? userText.implicitHeight + 24 : markdownView.implicitHeight + 28)
                        radius: 12

                        anchors.top: parent.top
                        anchors.left: isUser ? undefined : avatar.right
                        anchors.right: isUser ? avatar.left : undefined
                        anchors.leftMargin: isUser ? 0 : 14
                        anchors.rightMargin: isUser ? 14 : 0

                        color: isUser ? "#89b4fa" : (isThinking ? "#181825" : "#26273a")
                        border.color: isUser ? "#89b4fa" : (isSystem ? "#f38ba8" : "#3b3d54")
                        border.width: isUser ? 0 : 1

                        // Estado de Sistema / Pensando con icono vectorial puro (0 emojis)
                        RowLayout {
                            id: statusRow
                            visible: isSystem || isThinking
                            x: 16
                            anchors.verticalCenter: parent.verticalCenter
                            width: Math.min(bubble.parent.width * 0.75, statusText.implicitWidth + 26)
                            spacing: 8

                            Icon {
                                iconName: isThinking ? "hourglass" : "warning"
                                iconColor: isThinking ? "#cba6f7" : "#f38ba8"
                                iconSize: 14
                                Layout.alignment: Qt.AlignVCenter
                            }

                            Text {
                                id: statusText
                                Layout.fillWidth: true
                                text: modelData.replace("Sistema: ", "").replace("Agente: ", "")
                                color: isThinking ? "#a6adc8" : "#f38ba8"
                                font.italic: isThinking
                                font.pixelSize: 13
                                wrapMode: Text.Wrap
                            }
                        }

                        Text {
                            id: userText
                            visible: isUser
                            x: 14; y: 12
                            width: parent.width - 28
                            text: modelData.replace("Tú: ", "")
                            color: "#11111b"
                            font.pixelSize: 14
                            font.weight: Font.Medium
                            wrapMode: Text.Wrap
                            lineHeight: 1.25
                        }

                        MarkdownView {
                            id: markdownView
                            visible: isMarkdown
                            x: 14; y: 14
                            width: parent.width - 28
                            markdownText: isMarkdown ? modelData.replace("Agente: ", "") : ""
                        }
                    }
                }
            }
        }

        ChatInput {
            Layout.fillWidth: true
        }
    }
}
