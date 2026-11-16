import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "../components"
import FronterMind.Components.MarkdownViewer

Item {
    id: root

    // ========================================================================
    // SECCIÓN 1: DIÁLOGO NATIVO DE SELECCIÓN DE PROYECTO (WORKSPACE)
    // ========================================================================
    FolderDialog {
        id: workspaceDialog
        title: "Seleccionar carpeta del proyecto (Workspace)"
        onAccepted: backend.setWorkspacePath(selectedFolder.toString())
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ====================================================================
        // SECCIÓN 2: CABECERA SUPERIOR (TÍTULO, PRESUPUESTO DE TOKENS Y MODELO)
        // ====================================================================
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 72
            color: theme.bgBase

            // Línea divisoria inferior de la cabecera
            Rectangle {
                width: parent.width
                height: 1
                color: theme.borderColor
                anchors.bottom: parent.bottom
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 30
                anchors.rightMargin: 30
                spacing: 16

                // --- 2.1: Título de la Vista y Medidor de Contexto (Tokens) ---
                ColumnLayout {
                    spacing: 4
                    Text {
                        text: "Agente Asistente"
                        color: theme.textPrimary
                        font.family: theme.fontFamily
                        font.pixelSize: 18
                        font.bold: true
                    }

                    RowLayout {
                        spacing: 10
                        Icon { iconName: "database"; iconColor: theme.accentColor; iconSize: 11 }
                        Text {
                            text: backend.usedTokens + " / " + backend.maxContext + " tokens"
                            color: theme.textSecondary
                            font.family: theme.monoFontFamily
                            font.pixelSize: 11
                        }
                        Rectangle {
                            width: 60; height: 5; radius: 3
                            color: theme.bgSurface
                            Rectangle {
                                width: Math.min(parent.width, (backend.usedTokens / Math.max(1, backend.maxContext)) * parent.width)
                                height: parent.height
                                radius: 3
                                color: (backend.usedTokens / Math.max(1, backend.maxContext)) > 0.85 ? "#f38ba8" : theme.accentColor
                                Behavior on width { NumberAnimation { duration: 180 } }
                            }
                        }
                    }
                }

                // --- 2.2: Botón Selector de Carpeta de Trabajo (Workspace) ---
                Rectangle {
                    Layout.preferredHeight: 34
                    Layout.preferredWidth: Math.min(320, wsRow.implicitWidth + 24)
                    radius: 8
                    color: wsMouse.containsMouse ? theme.bgSurface : theme.bgSidebar
                    border.color: wsMouse.containsMouse ? theme.accentColor : theme.borderColor
                    border.width: 1

                    RowLayout {
                        id: wsRow
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        spacing: 8

                        Icon { iconName: "folder-open"; iconColor: theme.accentColor; iconSize: 14 }
                        Text {
                            text: backend.workspacePath
                            color: theme.textPrimary
                            font.family: theme.monoFontFamily
                            font.pixelSize: 11
                            elide: Text.ElideLeft
                            Layout.fillWidth: true
                        }
                    }

                    MouseArea {
                        id: wsMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: workspaceDialog.open()
                    }
                }

                // Espaciador elástico central
                Item { Layout.fillWidth: true }

                // --- 2.3: Selector Desplegable de Modelo GGUF Activo ---
                Text {
                    text: "Modelo:"
                    color: theme.textSecondary
                    font.family: theme.fontFamily
                    font.pixelSize: 13
                }

                // --- 2.3: Selector Desplegable de Modelo GGUF Activo (Sincronizado en tiempo real) ---
                ComboBox {
                    id: modelSelector
                    Layout.preferredWidth: 220
                    Layout.preferredHeight: 36
                    model: backend.localModels
                    textRole: "alias"

                    function syncCurrentModelIndex() {
                        for (var i = 0; i < backend.localModels.length; ++i) {
                            var item = backend.localModels[i]
                            if (backend.currentModel.indexOf(item.alias + " (") === 0) {
                                currentIndex = i
                                return
                            }
                        }
                    }

                    Component.onCompleted: syncCurrentModelIndex()
                    Connections {
                        target: backend
                        function onCurrentModelChanged() { modelSelector.syncCurrentModelIndex() }
                        function onLocalModelsChanged() { modelSelector.syncCurrentModelIndex() }
                    }

                    onActivated: (index) => {
                        var m = backend.localModels[index]
                        backend.loadModel(m.alias, m.path)
                    }
                    contentItem: Text {
                        leftPadding: 12
                        rightPadding: 24
                        text: modelSelector.currentIndex >= 0 ? modelSelector.textAt(modelSelector.currentIndex) : "Seleccionar modelo"
                        color: theme.textPrimary
                        font.family: theme.fontFamily
                        font.pixelSize: 13
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: theme.bgSidebar
                        border.color: modelSelector.hovered ? theme.accentColor : theme.borderColor
                        radius: 8
                    }
                }

                // --- 2.4: Botón para Limpiar Historial de Conversación ---
                Rectangle {
                    Layout.preferredWidth: 36
                    Layout.preferredHeight: 36
                    radius: 8
                    color: clearMouse.containsMouse ? theme.bgSurface : theme.bgSidebar
                    border.color: clearMouse.containsMouse ? "#f38ba8" : theme.borderColor
                    border.width: 1

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

        // ====================================================================
        // SECCIÓN 3: CONTENEDOR CENTRAL DEL CHAT (BIENVENIDA + LISTA DE MENSAJES)
        // ====================================================================
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: theme.bgBase

            // --- 3.1: Pantalla de Bienvenida (Visible cuando no hay mensajes) ---
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 14
                visible: backend.chatMessages.length === 0

                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    width: 72; height: 72; radius: 18
                    color: "#11111b"
                    border.color: theme.accentColor
                    border.width: 1
                    clip: true

                    Image {
                        anchors.fill: parent
                        anchors.margins: 4
                        sourceSize: Qt.size(128, 128)
                        source: "qrc:/assets/logo.svg"
                        fillMode: Image.PreserveAspectCrop
                        smooth: true
                    }
                }

                Text {
                    text: "¿En qué estamos trabajando hoy?"
                    color: theme.textPrimary
                    font.family: theme.fontFamily
                    font.pixelSize: 24
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: "Proyecto activo: " + backend.workspacePath
                    color: theme.textSecondary
                    font.family: theme.monoFontFamily
                    font.pixelSize: 12
                    Layout.alignment: Qt.AlignHCenter
                }
            }

            // --- 3.2: Lista Dinámica de Mensajes y Trazas del Agente ---
            ListView {
                id: messageList
                anchors.fill: parent
                clip: true
                spacing: 12
                cacheBuffer: 2000
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

                    // --- 3.2.1: Clasificación del Tipo de Mensaje ---
                    readonly property bool isUser: modelData.startsWith("Tú:")
                    readonly property bool isTool: modelData.startsWith("Tool:")
                    readonly property bool isSystem: modelData.startsWith("Sistema:")
                    readonly property bool isThinking: modelData.startsWith("Agente: ") && modelData.endsWith("...")
                    readonly property bool isMarkdown: !isUser && !isSystem && !isThinking && !isTool

                    // --- 3.2.2: Perfil de Avatar (Usuario a la derecha / IA a la izquierda) ---
                    Rectangle {
                        id: avatar
                        visible: !isTool
                        width: 34; height: 34; radius: 9
                        color: isUser ? theme.userBubbleColor : "#11111b"
                        border.color: isUser ? "transparent" : theme.accentColor
                        border.width: isUser ? 0 : 1
                        clip: true
                        anchors.left: isUser ? undefined : parent.left
                        anchors.right: isUser ? parent.right : undefined
                        anchors.top: parent.top

                        Icon {
                            visible: isUser
                            anchors.centerIn: parent
                            iconName: "user"
                            iconColor: "#11111b"
                            iconSize: 16
                        }

                        Image {
                            visible: !isUser
                            anchors.fill: parent
                            anchors.margins: 2
                            sourceSize: Qt.size(64, 64)
                            source: "qrc:/assets/logo.svg"
                            fillMode: Image.PreserveAspectCrop
                            smooth: true
                        }
                    }

                    // --- 3.2.3: Contenedor de Burbuja / Traza de Actividad ---
                    Rectangle {
                        id: bubble
                        readonly property real maxBubbleWidth: parent.width * 0.85

                        width: (isSystem || isThinking || isTool)
                               ? Math.min(maxBubbleWidth, statusRow.implicitWidth + 28)
                               : (isUser
                                  ? Math.min(parent.width * 0.78, userText.implicitWidth + 28)
                                  : Math.min(maxBubbleWidth, markdownView.implicitWidth + 28))
                        height: (isSystem || isThinking || isTool)
                                ? Math.max(isTool ? 28 : 36, statusText.implicitHeight + (isTool ? 10 : 18))
                                : (isUser ? userText.implicitHeight + 24 : markdownView.implicitHeight + 28)
                        radius: isTool ? 6 : 12

                        anchors.top: parent.top
                        anchors.left: isUser ? undefined : (isTool ? parent.left : avatar.right)
                        anchors.right: isUser ? avatar.left : undefined
                        anchors.leftMargin: isUser ? 0 : (isTool ? 48 : 14)
                        anchors.rightMargin: isUser ? 14 : 0

                        color: isUser
                               ? theme.userBubbleColor
                               : (isTool ? theme.bgSidebar : (isThinking ? theme.bgSidebar : theme.bgSurface))
                        border.color: isUser
                                      ? theme.userBubbleColor
                                      : (isSystem ? "#f38ba8" : (isTool ? theme.borderColor : theme.borderColor))
                        border.width: isUser ? 0 : 1

                        // --- 3.2.4: Vista de Estado / Monólogo Interno / Paso de Herramienta ---
                        RowLayout {
                            id: statusRow
                            visible: isSystem || isThinking || isTool
                            x: 12
                            anchors.verticalCenter: parent.verticalCenter
                            width: Math.min(bubble.maxBubbleWidth - 24, statusText.implicitWidth + 26)
                            spacing: 8

                            // Indicador visual de actividad (Punto verde en pasos completados / Icono en pensamiento)
                            Rectangle {
                                visible: isTool
                                width: 6; height: 6; radius: 3
                                color: "#a6e3a1"
                                Layout.alignment: Qt.AlignVCenter
                            }

                            Icon {
                                visible: !isTool
                                iconName: isThinking ? "hourglass" : "warning"
                                iconColor: isThinking ? theme.accentColor : "#f38ba8"
                                iconSize: 14
                                Layout.alignment: Qt.AlignVCenter
                            }

                            Text {
                                id: statusText
                                Layout.fillWidth: true
                                text: modelData.replace("Sistema: ", "").replace("Agente: ", "").replace("Tool: ", "")
                                color: isTool ? theme.textSecondary : (isThinking ? theme.textPrimary : "#f38ba8")
                                font.family: isTool ? theme.monoFontFamily : theme.fontFamily
                                font.italic: isThinking
                                font.pixelSize: isTool ? 12 : 13
                                wrapMode: Text.Wrap
                            }
                        }

                        // --- 3.2.5: Texto del Mensaje del Usuario ---
                        Text {
                            id: userText
                            visible: isUser
                            x: 14; y: 12
                            width: parent.width - 28
                            text: modelData.replace("Tú: ", "")
                            color: "#11111b"
                            font.family: theme.fontFamily
                            font.pixelSize: theme.baseFontSize
                            font.weight: Font.Medium
                            wrapMode: Text.Wrap
                            lineHeight: 1.25
                        }

                        // --- 3.2.6: Renderizador C++ Markdown para Respuestas del Agente ---
                        MarkdownView {
                            id: markdownView
                            visible: isMarkdown
                            x: 14; y: 14
                            maxAvailableWidth: bubble.maxBubbleWidth - 28
                            width: bubble.width - 28
                            markdownText: isMarkdown ? modelData.replace("Agente: ", "") : ""
                        }
                    }
                }
            }
        }

        // ====================================================================
        // SECCIÓN 4: BARRA INFERIOR DE ENTRADA DE MENSAJES Y ARCHIVOS ADJUNTOS
        // ====================================================================
        ChatInput {
            Layout.fillWidth: true
        }
    }
}
