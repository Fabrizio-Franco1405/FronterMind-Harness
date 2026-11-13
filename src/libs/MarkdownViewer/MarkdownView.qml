import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property string markdownText: ""

    implicitHeight: layout.implicitHeight

    MarkdownParser {
        id: bridge
        markdown: root.markdownText
    }

    ColumnLayout {
        id: layout
        width: root.width
        spacing: 8

        Repeater {
            model: bridge.blocks

            delegate: Item {
                id: delegateRoot
                Layout.fillWidth: true
                Layout.topMargin: isCode ? 4 : 0
                Layout.bottomMargin: isCode ? 4 : 0
                implicitHeight: isCode ? codeContainer.implicitHeight : richTextItem.implicitHeight

                readonly property var blockData: modelData
                readonly property bool isCode: blockData.type === "code"

                // --- BLOQUE DE TEXTO (MARKDOWN ESTILIZADO) ---
                TextEdit {
                    id: richTextItem
                    visible: !delegateRoot.isCode
                    width: parent.width
                    text: !delegateRoot.isCode ? (delegateRoot.blockData.content || "") : ""
                    textFormat: TextEdit.RichText
                    color: "#cdd6f4"
                    font.pixelSize: 14
                    wrapMode: TextEdit.Wrap
                    readOnly: true
                    selectByMouse: true
                    selectedTextColor: "#1e1e2e"
                    selectionColor: "#cba6f7"
                    onLinkActivated: (link) => Qt.openUrlExternally(link)
                }

                // --- TARJETA DE CÓDIGO (ESTILO CATPPUCCIN MOCHA) ---
                Rectangle {
                    id: codeContainer
                    visible: delegateRoot.isCode
                    width: parent.width
                    implicitHeight: codeColumn.implicitHeight
                    color: "#181825" // Mismo tono oscuro elegante que tu barra de input
                    radius: 10
                    border.color: "#45475a"
                    border.width: 1

                    property bool copied: false

                    Timer {
                        id: copyResetTimer
                        interval: 1800
                        onTriggered: codeContainer.copied = false
                    }

                    ColumnLayout {
                        id: codeColumn
                        width: parent.width
                        spacing: 0

                        // Cabecera superior de la tarjeta de código
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 36
                            color: "#11111b"
                            radius: 10

                            // Parche inferior para que solo las esquinas de arriba sean redondeadas
                            Rectangle {
                                width: parent.width
                                height: 10
                                color: "#11111b"
                                anchors.bottom: parent.bottom
                            }

                            // Línea divisoria sutil
                            Rectangle {
                                width: parent.width
                                height: 1
                                color: "#313244"
                                anchors.bottom: parent.bottom
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 14
                                anchors.rightMargin: 12
                                spacing: 8

                                // Puntito decorativo con el acento lavanda de FronterMind
                                Rectangle {
                                    width: 8
                                    height: 8
                                    radius: 4
                                    color: "#cba6f7"
                                }

                                // Etiqueta del lenguaje
                                Text {
                                    text: (delegateRoot.blockData.language && delegateRoot.blockData.language !== "")
                                          ? delegateRoot.blockData.language.toUpperCase()
                                          : "CÓDIGO"
                                    color: "#a6adc8"
                                    font.pixelSize: 11
                                    font.bold: true
                                    font.family: "monospace"
                                    font.letterSpacing: 0.8
                                    Layout.fillWidth: true
                                }

                                // Botón Copiar tipo píldora
                                Rectangle {
                                    Layout.preferredWidth: copyLabel.implicitWidth + 20
                                    Layout.preferredHeight: 24
                                    radius: 6
                                    color: codeContainer.copied
                                           ? "#1e3a2f"
                                           : (copyMouse.containsMouse ? "#313244" : "#1e1e2e")
                                    border.color: codeContainer.copied ? "#a6e3a1" : "#45475a"
                                    border.width: 1
                                    Behavior on color { ColorAnimation { duration: 120 } }

                                    Text {
                                        id: copyLabel
                                        anchors.centerIn: parent
                                        text: codeContainer.copied ? "✓ Copiado" : "Copiar código"
                                        color: codeContainer.copied ? "#a6e3a1" : "#cdd6f4"
                                        font.pixelSize: 11
                                        font.bold: codeContainer.copied
                                    }

                                    MouseArea {
                                        id: copyMouse
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: {
                                            bridge.copyToClipboard(delegateRoot.blockData.content || "")
                                            codeContainer.copied = true
                                            copyResetTimer.restart()
                                        }
                                    }
                                }
                            }
                        }

                        // Área del código con padding holgado
                        Item {
                            Layout.fillWidth: true
                            Layout.preferredHeight: codeTextEdit.implicitHeight + 28

                            TextEdit {
                                id: codeTextEdit
                                x: 16
                                y: 14
                                width: parent.width - 32
                                text: delegateRoot.isCode ? (delegateRoot.blockData.highlighted || "") : ""
                                textFormat: TextEdit.RichText
                                color: "#cdd6f4"
                                font.family: "monospace"
                                font.pixelSize: 13
                                wrapMode: TextEdit.WrapAnywhere
                                readOnly: true
                                selectByMouse: true
                                selectedTextColor: "#1e1e2e"
                                selectionColor: "#89b4fa"
                            }
                        }
                    }
                }
            }
        }
    }
}
