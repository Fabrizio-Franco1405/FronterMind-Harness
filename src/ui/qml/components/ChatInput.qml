import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "../components"

Rectangle {
    id: root
    color: "#1e1e2e"

    readonly property bool canSend: (chatInput.text.trim() !== "" || backend.attachedFileName !== "") && !backend.isGenerating
    implicitHeight: inputCard.implicitHeight + 32

    function submitMessage() {
        if (root.canSend) {
            backend.sendMessage(chatInput.text)
            chatInput.text = ""
        }
    }

    Rectangle {
        id: inputCard
        anchors.fill: parent
        anchors.leftMargin: 30
        anchors.rightMargin: 30
        anchors.topMargin: 12
        anchors.bottomMargin: 20
        implicitHeight: Math.min(180, Math.max(54, innerColumn.implicitHeight + 20))
        color: "#181825"
        radius: 14
        border.color: chatInput.activeFocus ? "#cba6f7" : "#313244"
        border.width: chatInput.activeFocus ? 1.5 : 1
        Behavior on border.color { ColorAnimation { duration: 100 } }

        ColumnLayout {
            id: innerColumn
            anchors.fill: parent
            anchors.margins: 10
            spacing: 8

            // --- CHIP DE ARCHIVO ADJUNTO (CON ICONOS PUROS) ---
            Rectangle {
                visible: backend.attachedFileName !== ""
                Layout.fillWidth: true
                Layout.preferredHeight: 30
                color: "#313244"
                radius: 8
                border.color: "#45475a"
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 8
                    spacing: 8

                    Icon { iconName: "file"; iconColor: "#cba6f7"; iconSize: 13 }

                    Text {
                        text: backend.attachedFileName
                        color: "#cdd6f4"
                        font.pixelSize: 12
                        font.bold: true
                        elide: Text.ElideMiddle
                        Layout.fillWidth: true
                    }

                    Rectangle {
                        width: 22; height: 22; radius: 4
                        color: removeAttachMouse.containsMouse ? "#45475a" : "transparent"

                        Icon {
                            anchors.centerIn: parent
                            iconName: "close"
                            iconColor: "#f38ba8"
                            iconSize: 12
                        }

                        MouseArea {
                            id: removeAttachMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: backend.clearAttachment()
                        }
                    }
                }
            }

            // --- FILA DE ENTRADA ---
            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 10

                Rectangle {
                    Layout.preferredWidth: 36
                    Layout.preferredHeight: 36
                    Layout.alignment: Qt.AlignBottom
                    radius: 8
                    color: attachMouse.containsMouse ? "#313244" : "transparent"
                    Behavior on color { ColorAnimation { duration: 100 } }

                    Icon {
                        anchors.centerIn: parent
                        iconName: "attachement"
                        iconColor: attachMouse.containsMouse ? "#cdd6f4" : "#6c7086"
                        iconSize: 17
                    }

                    MouseArea {
                        id: attachMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: fileDialog.open()
                    }
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 36
                    Layout.maximumHeight: 120
                    clip: true

                    TextArea {
                        id: chatInput
                        placeholderText: backend.isGenerating
                                         ? "El agente está procesando tu solicitud..."
                                         : "Escribe un mensaje o pega código... (Enter para enviar, Shift+Enter nueva línea)"
                        placeholderTextColor: "#585b70"
                        color: "#cdd6f4"
                        font.pixelSize: 14
                        wrapMode: TextArea.Wrap
                        background: null
                        enabled: !backend.isGenerating
                        topPadding: 7
                        bottomPadding: 7

                        Keys.onPressed: function(event) {
                            if ((event.key === Qt.Key_Return || event.key === Qt.Key_Enter) && !(event.modifiers & Qt.ShiftModifier)) {
                                event.accepted = true
                                root.submitMessage()
                            }
                        }
                    }
                }

                Rectangle {
                    id: sendBtn
                    Layout.preferredWidth: 38
                    Layout.preferredHeight: 38
                    Layout.alignment: Qt.AlignBottom
                    radius: 9
                    color: root.canSend ? (sendMouse.containsMouse ? "#b4befe" : "#cba6f7") : "#242436"
                    Behavior on color { ColorAnimation { duration: 100 } }

                    Icon {
                        anchors.centerIn: parent
                        anchors.horizontalCenterOffset: 1
                        iconName: "send"
                        iconColor: root.canSend ? "#11111b" : "#585b70"
                        iconSize: 17
                    }

                    MouseArea {
                        id: sendMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: root.canSend ? Qt.PointingHandCursor : Qt.ArrowCursor
                        onClicked: root.submitMessage()
                    }
                }
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: "Adjuntar archivo al contexto"
        nameFilters: ["Archivos de código y texto (*.txt *.md *.json *.py *.cpp *.h *.hpp *.qml *.cmake)", "Todos los archivos (*)"]
        onAccepted: backend.attachFile(selectedFile.toString().replace("file://", ""))
    }
}
