import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "../components"
import FronterMind.Components.MarkdownViewer

Item {
    id: root

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // --- HEADER ---
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 70
            color: "#1e1e2e"

            Rectangle { width: parent.width; height: 1; color: "#313244"; anchors.bottom: parent.bottom }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 30
                anchors.rightMargin: 30
                spacing: 20

                Column {
                    Text { text: "Agente Asistente"; color: "#cdd6f4"; font.pixelSize: 18; font.bold: true }
                    Row {
                        spacing: 5
                        Icon { iconName: "database"; iconColor: "#6c7086"; iconSize: 10; anchors.verticalCenter: parent.verticalCenter }
                        Text {
                            text: backend.usedTokens + " / " + backend.maxContext + " tokens";
                            color: "#6c7086"
                            font.pixelSize: 11
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }

                Item { Layout.fillWidth: true }

                Text { text: "Modelo:"; color: "#a6adc8"; font.pixelSize: 13; verticalAlignment: Text.AlignVCenter }
                ComboBox {
                    Layout.preferredWidth: 260
                    Layout.preferredHeight: 36
                    model: backend.localModels
                    textRole: "alias"
                    onActivated: {
                        var model = backend.localModels[currentIndex];
                        backend.loadModel(model.alias, model.path);
                    }
                    contentItem: Text {
                        leftPadding: 12
                        text: parent.currentIndex >= 0 ? parent.textAt(parent.currentIndex) : "Seleccionar modelo"
                        color: "#cdd6f4"
                        font.pixelSize: 13
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle { color: "#181825"; border.color: "#45475a"; radius: 8 }
                }
                // BOTÓN PARA LIMPIAR EL CHAT
                Item {
                    Layout.preferredWidth: 36
                    Layout.preferredHeight: 36
                    Rectangle {
                        anchors.fill: parent
                        radius: 6
                        color: "transparent"
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: backend.clearChat()
                        }
                        Icon {
                            anchors.centerIn: parent;
                            iconName: "trash";
                            iconColor: "#f38ba8";
                            iconSize: 16
                        }
                    }
                }
            }
        }

        // --- ÁREA DE MENSAJES ---
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Column {
                anchors.centerIn: parent
                spacing: 20
                visible: backend.chatMessages.length === 0

                Icon {
                    iconName: "stars"
                    iconColor: "#cba6f7"
                    iconSize: 64
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                Text {
                    text: "¿En qué estás trabajando hoy?"
                    color: "#cdd6f4"
                    font.pixelSize: 24
                    font.bold: true
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                Text {
                    text: "Selecciona un modelo y empieza a chatear con tu IA local."
                    color: "#6c7086"
                    font.pixelSize: 14
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }

            ListView {
                id: messageList
                anchors.fill: parent
                clip: true
                spacing: 25
                model: backend.chatMessages
                visible: backend.chatMessages.length > 0

                onCountChanged: positionViewAtEnd()
                topMargin: 30; bottomMargin: 30; leftMargin: 30; rightMargin: 30

                delegate: Item {
                    width: messageList.width - 60
                    height: bubble.height

                    property bool isUser: modelData.startsWith("Tú:")
                    property bool isThinking: modelData === "Agente: Pensando..." || modelData.startsWith("Agente: Pensando")
                    property bool isSystem: modelData.startsWith("Sistema:")
                    property bool isMarkdown: !isUser && !isSystem && !isThinking

                    Rectangle {
                        id: avatar
                        width: 32; height: 32; radius: 16
                        color: isUser ? "#89b4fa" : "#cba6f7"
                        anchors.left: isUser ? undefined : parent.left
                        anchors.right: isUser ? parent.right : undefined
                        anchors.top: parent.top

                        Icon {
                            anchors.centerIn: parent
                            iconName: isUser ? "user" : "tape"
                            iconColor: "#1e1e2e"
                            iconSize: 16
                        }
                    }

                    Rectangle {
                        id: bubble
                        width: (isSystem || isThinking)
                                ? systemText.implicitWidth + 28
                                : (isUser ? Math.min(parent.width * 0.8, userText.implicitWidth + 24) : parent.width * 0.85)
                        height: (isSystem || isThinking)
                                ? systemText.implicitHeight + 24
                                : (isUser ? userText.implicitHeight + 24 : markdownView.implicitHeight + 24)
                        radius: 12

                        anchors.top: parent.top
                        anchors.left: isUser ? undefined : avatar.right
                        anchors.right: isUser ? avatar.left : undefined
                        anchors.leftMargin: isUser ? 0 : 15
                        anchors.rightMargin: isUser ? 15 : 0

                        color: isUser ? "#89b4fa" : "#313244"
                        border.color: isUser ? "#89b4fa" : "#45475a"
                        border.width: isUser ? 0 : 1

                        Text {
                            id: systemText
                            visible: isSystem || isThinking
                            x: 14; y: 12
                            width: parent.width - 28
                            text: modelData.replace("Sistema: ", "").replace("Agente: ", "")
                            // Gris suave para "Pensando...", rojo suave solo para avisos del Sistema
                            color: isThinking ? "#a6adc8" : "#f38ba8"
                            font.italic: isThinking
                            font.pixelSize: 14
                            wrapMode: Text.Wrap
                        }

                        Text {
                            id: userText
                            visible: isUser
                            x: 12; y: 12
                            width: parent.width - 24
                            text: modelData.replace("Tú: ", "")
                            color: "#1e1e2e"
                            font.pixelSize: 14
                            wrapMode: Text.Wrap
                            lineHeight: 1.2
                        }

                        MarkdownView {
                            id: markdownView
                            visible: isMarkdown
                            x: 12; y: 12
                            width: parent.width - 24
                            markdownText: isMarkdown ? modelData.replace("Agente: ", "") : ""
                        }
                    }
                }
            }
        }

        // --- ÁREA DE INPUT (Con soporte para archivos) ---
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: backend.attachedFileName !== "" ? 130 : 90
            color: "#1e1e2e"
            Behavior on Layout.preferredHeight { NumberAnimation { duration: 150 } }

            Rectangle {
                anchors.fill: parent; anchors.margins: 20
                color: "#181825"; radius: 12
                border.color: chatInput.activeFocus ? "#cba6f7" : "#313244"
                border.width: 1
                Behavior on border.color { ColorAnimation { duration: 150 } }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 5

                    // CHIP DEL ARCHIVO ADJUNTO
                    Rectangle {
                        visible: backend.attachedFileName !== ""
                        Layout.fillWidth: true
                        Layout.preferredHeight: 30
                        color: "#313244"
                        radius: 6

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10

                            Text {
                                text: "📎 " + backend.attachedFileName;
                                color: "#cdd6f4";
                                font.pixelSize: 12;
                                Layout.fillWidth: true
                            }

                            Text {
                                text: "✕";
                                color: "#f38ba8";
                                font.pixelSize: 14;
                                font.bold: true
                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: backend.clearAttachment()
                                }
                            }
                        }
                    }

                    // FILA DE INPUT
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 10

                        // Botón Clip (Abre el FileDialog)
                        Item {
                            Layout.preferredWidth: 36
                            Layout.fillHeight: true
                            Rectangle {
                                anchors.centerIn: parent
                                width: 36; height: 36; radius: 6
                                color: "transparent"
                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: fileDialog.open()
                                }
                                Icon { anchors.centerIn: parent; iconName: "attachement"; iconColor: "#6c7086"; iconSize: 18 }
                            }
                        }

                        // Input de Texto
                        ScrollView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true

                            TextArea {
                                id: chatInput
                                placeholderText: backend.isGenerating ? "El agente está escribiendo..." : "Escribe un mensaje a FronterMind..."
                                color: "#cdd6f4"; font.pixelSize: 14; wrapMode: TextArea.Wrap; background: null
                                enabled: !backend.isGenerating
                                Keys.onPressed: function(event) {
                                    if ((event.key === Qt.Key_Return || event.key === Qt.Key_Enter) && !(event.modifiers & Qt.ShiftModifier)) {
                                        event.accepted = true; sendBtn.clicked();
                                    }
                                }
                            }
                        }

                        // Botón Enviar (Habilitado si hay texto O un archivo)
                        Item {
                            Layout.preferredWidth: 40
                            Layout.fillHeight: true
                            Rectangle {
                                anchors.centerIn: parent
                                width: 40; height: 40; radius: 8
                                color: ((chatInput.text.trim() !== "" || backend.attachedFileName !== "") && !backend.isGenerating) ? "#cba6f7" : "#313244"

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        if ((chatInput.text.trim() !== "" || backend.attachedFileName !== "") && !backend.isGenerating) {
                                            backend.sendMessage(chatInput.text);
                                            chatInput.text = "";
                                        }
                                    }
                                }

                                Icon {
                                    anchors.centerIn: parent
                                    anchors.horizontalCenterOffset: 1
                                    iconName: "send"
                                    iconColor: ((chatInput.text.trim() !== "" || backend.attachedFileName !== "") && !backend.isGenerating) ? "#1e1e2e" : "#6c7086"
                                    iconSize: 18
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // --- DIÁLOGO DE ARCHIVOS ---
    FileDialog {
        id: fileDialog
        title: "Adjuntar archivo al chat"
        nameFilters: ["Archivos de texto (*.txt *.md *.json *.py *.cpp *.h *.qml)", "Todos los archivos (*)"]
        onAccepted: {
            backend.attachFile(selectedFile.toString().replace("file://", ""))
        }
    }
}
