import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "../components"
import FronterMind.Components.MarkdownViewer

Item {
    id: root

    FolderDialog {
        id: workspaceDialog
        title: "Seleccionar carpeta del proyecto (Workspace)"
        onAccepted: backend.setWorkspacePath(selectedFolder.toString())
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // --- HEADER SUPERIOR ---
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 72
            color: theme.bgBase

            Rectangle { width: parent.width; height: 1; color: theme.bgSurface; anchors.bottom: parent.bottom }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 30
                anchors.rightMargin: 30
                spacing: 16

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

                // --- SELECTOR DE CARPETA DE PROYECTO (WORKSPACE) ---
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

                Item { Layout.fillWidth: true }

                Text { text: "Modelo:"; color: theme.textSecondary; font.family: theme.fontFamily; font.pixelSize: 13 }

                ComboBox {
                    id: modelSelector
                    Layout.preferredWidth: 220
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
                    color: theme.bgSidebar
                    border.color: theme.borderColor
                    border.width: 1

                    Icon {
                        anchors.centerIn: parent
                        iconName: "stars"
                        iconColor: theme.accentColor
                        iconSize: 36
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

            ListView {
                id: messageList
                anchors.fill: parent
                clip: true
                spacing: 16
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

                    property bool isUser: modelData.startsWith("Tú:")
                    property bool isTool: modelData.startsWith("Tool:")
                    property bool isThinking: modelData.startsWith("Agente: Pensando") || modelData.startsWith("Agente: Analizando")
                    property bool isSystem: modelData.startsWith("Sistema:")
                    property bool isMarkdown: !isUser && !isSystem && !isThinking && !isTool

                    Rectangle {
                        id: avatar
                        visible: !isTool
                        width: 34; height: 34; radius: 10
                        color: isUser ? theme.userBubbleColor : theme.bgSidebar
                        border.color: isUser ? "transparent" : theme.accentColor
                        border.width: isUser ? 0 : 1
                        anchors.left: isUser ? undefined : parent.left
                        anchors.right: isUser ? parent.right : undefined
                        anchors.top: parent.top

                        Icon {
                            anchors.centerIn: parent
                            iconName: isUser ? "user" : "stars"
                            iconColor: isUser ? "#11111b" : theme.accentColor
                            iconSize: 16
                        }
                    }

                    Rectangle {
                        id: bubble
                        readonly property real maxBubbleWidth: parent.width * 0.85

                        width: (isSystem || isThinking || isTool)
                               ? Math.min(maxBubbleWidth, statusRow.implicitWidth + 32)
                               : (isUser
                                  ? Math.min(parent.width * 0.78, userText.implicitWidth + 28)
                                  : Math.min(maxBubbleWidth, markdownView.implicitWidth + 28))
                        height: (isSystem || isThinking || isTool)
                                ? Math.max(36, statusText.implicitHeight + 18)
                                : (isUser ? userText.implicitHeight + 24 : markdownView.implicitHeight + 28)
                        radius: isTool ? 8 : 12

                        anchors.top: parent.top
                        anchors.left: isUser ? undefined : (isTool ? parent.left : avatar.right)
                        anchors.right: isUser ? avatar.left : undefined
                        anchors.leftMargin: isUser ? 0 : (isTool ? 48 : 14)
                        anchors.rightMargin: isUser ? 14 : 0

                        color: isUser ? theme.userBubbleColor : (isTool ? theme.bgDarkest : (isThinking ? theme.bgSidebar : theme.bgSurface))
                        border.color: isUser ? theme.userBubbleColor : (isSystem ? "#f38ba8" : (isTool ? theme.accentColor : theme.borderColor))
                        border.width: isUser ? 0 : 1

                        RowLayout {
                            id: statusRow
                            visible: isSystem || isThinking || isTool
                            x: 14
                            anchors.verticalCenter: parent.verticalCenter
                            width: Math.min(bubble.maxBubbleWidth - 28, statusText.implicitWidth + 28)
                            spacing: 8

                            Icon {
                                iconName: isTool ? "code" : (isThinking ? "hourglass" : "warning")
                                iconColor: isTool ? "#a6e3a1" : (isThinking ? theme.accentColor : "#f38ba8")
                                iconSize: 14
                                Layout.alignment: Qt.AlignVCenter
                            }

                            Text {
                                id: statusText
                                Layout.fillWidth: true
                                text: modelData.replace("Sistema: ", "").replace("Agente: ", "").replace("Tool: ", "")
                                color: isTool ? "#a6e3a1" : (isThinking ? theme.textSecondary : "#f38ba8")
                                font.family: isTool ? theme.monoFontFamily : theme.fontFamily
                                font.italic: isThinking
                                font.pixelSize: isTool ? 12 : 13
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
                            font.family: theme.fontFamily
                            font.pixelSize: theme.baseFontSize
                            font.weight: Font.Medium
                            wrapMode: Text.Wrap
                            lineHeight: 1.25
                        }

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

        ChatInput {
            Layout.fillWidth: true
        }
    }
}
