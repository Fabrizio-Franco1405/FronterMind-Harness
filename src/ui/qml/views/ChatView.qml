import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

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
                        Icon { iconCode: "\uf1c0"; iconColor: "#6c7086"; font.pixelSize: 10; anchors.verticalCenter: parent.verticalCenter }
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
                    iconCode: "\uf544" // Robot
                    iconColor: "#cba6f7"
                    font.pixelSize: 64
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
                    property bool isSystem: modelData.startsWith("Sistema:")

                    Rectangle {
                        id: avatar
                        width: 32; height: 32; radius: 16
                        color: isUser ? "#89b4fa" : "#cba6f7"
                        anchors.left: isUser ? undefined : parent.left
                        anchors.right: isUser ? parent.right : undefined
                        anchors.top: parent.top

                        Icon {
                            anchors.centerIn: parent
                            iconCode: isUser ? "\uf007" : "\uf544"
                            iconColor: "#1e1e2e"
                            font.pixelSize: 16
                        }
                    }

                    Rectangle {
                        id: bubble
                        width: Math.min(parent.width * 0.8, messageText.implicitWidth + 30)
                        height: messageText.implicitHeight + 24
                        radius: 12

                        anchors.top: parent.top
                        anchors.left: isUser ? undefined : avatar.right
                        anchors.right: isUser ? avatar.left : undefined
                        anchors.leftMargin: isUser ? 0 : 15
                        anchors.rightMargin: isUser ? 15 : 0

                        color: isSystem ? "transparent" : (isUser ? "#89b4fa" : "#313244")
                        border.color: isSystem ? "transparent" : (isUser ? "#89b4fa" : "#45475a")
                        border.width: isSystem ? 0 : 1

                        Text {
                            id: messageText
                            anchors.centerIn: parent
                            width: parent.width - 24
                            text: modelData.replace("Tú: ", "").replace("Agente: ", "").replace("Sistema: ", "")
                            color: isSystem ? "#f38ba8" : (isUser ? "#1e1e2e" : "#cdd6f4")
                            font.pixelSize: 14; wrapMode: Text.Wrap; lineHeight: 1.2
                        }
                    }
                }
            }
        }

        // --- ÁREA DE INPUT (Simplificada y a prueba de errores) ---
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 90
            color: "#1e1e2e"

            Rectangle {
                anchors.fill: parent; anchors.margins: 20
                color: "#181825"; radius: 12
                border.color: chatInput.activeFocus ? "#cba6f7" : "#313244"
                border.width: 1
                Behavior on border.color { ColorAnimation { duration: 150 } }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10

                    // Clip
                    Item {
                        Layout.preferredWidth: 36
                        Layout.fillHeight: true
                        Icon { anchors.centerIn: parent; iconCode: "\uf0c6"; iconColor: "#6c7086"; font.pixelSize: 18 }
                    }

                    // Input
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

                    // Botón Enviar
                    Item {
                        Layout.preferredWidth: 40
                        Layout.fillHeight: true
                        Rectangle {
                            anchors.centerIn: parent
                            width: 40; height: 40; radius: 8
                            color: (chatInput.text.trim() !== "" && !backend.isGenerating) ? "#cba6f7" : "#313244"

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    if (chatInput.text.trim() !== "" && !backend.isGenerating) {
                                        backend.sendMessage(chatInput.text);
                                        chatInput.text = "";
                                    }
                                }
                            }

                            Icon {
                                anchors.centerIn: parent
                                anchors.horizontalCenterOffset: 1 // Corrección óptica
                                iconCode: "\uf1d8"
                                iconColor: (chatInput.text.trim() !== "" && !backend.isGenerating) ? "#1e1e2e" : "#6c7086"
                                font.pixelSize: 18
                            }
                        }
                    }
                }
            }
        }
    }
}
