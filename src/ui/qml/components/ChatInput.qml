import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "../components"

Rectangle {
    id: root
    Layout.fillWidth: true
    implicitHeight: mainCol.implicitHeight + 24
    color: theme.bgBase   // <-- Antes tenía un color oscuro fijo que hacía parecer que estaba en otro contenedor

    // Línea divisoria superior sutil
    Rectangle {
        width: parent.width
        height: 1
        color: theme.borderColor
        anchors.top: parent.top
    }

    ColumnLayout {
        id: mainCol
        anchors.fill: parent
        anchors.margins: 12
        anchors.leftMargin: 24
        anchors.rightMargin: 24
        spacing: 8

        // Chip de archivo adjunto (si existe)
        Rectangle {
            visible: backend.attachedFileName !== ""
            Layout.preferredHeight: 28
            Layout.preferredWidth: attachRow.implicitWidth + 20
            color: theme.bgSidebar
            border.color: theme.accentColor
            border.width: 1
            radius: 6

            RowLayout {
                id: attachRow
                anchors.centerIn: parent
                spacing: 8
                Icon { iconName: "file"; iconColor: theme.accentColor; iconSize: 13 }
                Text {
                    text: backend.attachedFileName
                    color: theme.textPrimary
                    font.family: theme.monoFontFamily
                    font.pixelSize: 12
                }
                MouseArea {
                    width: 16; height: 16
                    cursorShape: Qt.PointingHandCursor
                    onClicked: backend.clearAttachment()
                    Icon { anchors.centerIn: parent; iconName: "close"; iconColor: "#f38ba8"; iconSize: 11 }
                }
            }
        }

        // Caja redondeada del Input
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: Math.min(Math.max(50, inputScroll.contentHeight + 20), 160)
            color: theme.bgSidebar
            border.color: messageInput.activeFocus ? theme.accentColor : theme.borderColor
            border.width: 1
            radius: 12
            Behavior on border.color { ColorAnimation { duration: 120 } }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 10
                spacing: 10

                // Botón Adjuntar Archivo
                Rectangle {
                    Layout.preferredWidth: 34
                    Layout.preferredHeight: 34
                    radius: 8
                    color: attachMouse.containsMouse ? theme.bgSurface : "transparent"

                    Icon {
                        anchors.centerIn: parent
                        iconName: "attachement"
                        iconColor: theme.textSecondary
                        iconSize: 16
                    }

                    MouseArea {
                        id: attachMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: attachDialog.open()
                    }
                }

                // Área de texto desplazable
                ScrollView {
                    id: inputScroll
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true

                    TextArea {
                        id: messageInput
                        placeholderText: "Escribe un mensaje o pega código... (Enter para enviar, Shift+Enter nueva línea)"
                        placeholderTextColor: theme.textSecondary
                        color: theme.textPrimary
                        font.family: theme.fontFamily
                        font.pixelSize: 13
                        wrapMode: TextArea.Wrap
                        verticalAlignment: TextEdit.AlignVCenter
                        background: null
                        selectByMouse: true

                        Keys.onReturnPressed: (event) => {
                            if (event.modifiers & Qt.ShiftModifier) {
                                event.accepted = false
                            } else {
                                event.accepted = true
                                root.submit()
                            }
                        }
                    }
                }

                // Botón Enviar
                Rectangle {
                    Layout.preferredWidth: 36
                    Layout.preferredHeight: 36
                    radius: 8
                    readonly property bool canSend: (messageInput.text.trim().length > 0 || backend.attachedFileName !== "") && !backend.isGenerating
                    color: canSend ? theme.accentColor : theme.bgSurface
                    border.color: theme.borderColor
                    border.width: canSend ? 0 : 1
                    Behavior on color { ColorAnimation { duration: 120 } }

                    Icon {
                        anchors.centerIn: parent
                        iconName: "send"
                        iconColor: parent.canSend ? "#11111b" : theme.textSecondary
                        iconSize: 15
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: parent.canSend ? Qt.PointingHandCursor : Qt.ArrowCursor
                        onClicked: if (parent.canSend) root.submit()
                    }
                }
            }
        }
    }

    function submit() {
        if (backend.isGenerating) return
        const txt = messageInput.text
        if (txt.trim() === "" && backend.attachedFileName === "") return
        messageInput.text = ""
        backend.sendMessage(txt)
    }

    FileDialog {
        id: attachDialog
        title: "Adjuntar archivo al contexto"
        onAccepted: backend.attachFile(selectedFile.toString())
    }
}
