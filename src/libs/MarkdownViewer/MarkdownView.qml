import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property string markdownText: ""
    property real maxAvailableWidth: 750

    // Calculamos el ancho ideal usando implicitWidth (ancho sin wrap) de cada bloque
    implicitWidth: {
        var maxW = 0
        for (var i = 0; i < blockRepeater.count; ++i) {
            var item = blockRepeater.itemAt(i)
            if (item) {
                maxW = Math.max(maxW, item.unwrappedWidth)
            }
        }
        return Math.min(root.maxAvailableWidth, Math.max(120, maxW))
    }
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
            id: blockRepeater
            model: bridge.blocks

            delegate: Item {
                id: delegateRoot
                Layout.fillWidth: true
                Layout.topMargin: isCode ? 4 : 0
                Layout.bottomMargin: isCode ? 4 : 0
                implicitHeight: isCode ? codeContainer.implicitHeight : richTextItem.contentHeight

                readonly property var blockData: modelData
                readonly property bool isCode: blockData.type === "code"

                // Si es código o tiene listas/múltiples párrafos, usa el ancho amplio;
                // si es una frase corta, usa implicitWidth + margen de seguridad.
                readonly property real unwrappedWidth: {
                    if (isCode) return root.maxAvailableWidth
                    var html = blockData.content || ""
                    if (html.indexOf("<ul") !== -1 || html.indexOf("<ol") !== -1 || html.indexOf("<table") !== -1) {
                        return Math.min(root.maxAvailableWidth, Math.max(480, richTextItem.implicitWidth + 24))
                    }
                    return Math.ceil(richTextItem.implicitWidth) + 16
                }

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

                Rectangle {
                    id: codeContainer
                    visible: delegateRoot.isCode
                    width: parent.width
                    implicitHeight: codeColumn.implicitHeight
                    color: "#181825"
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

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 36
                            color: "#11111b"
                            radius: 10

                            Rectangle {
                                width: parent.width
                                height: 10
                                color: "#11111b"
                                anchors.bottom: parent.bottom
                            }

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

                                Rectangle {
                                    width: 8
                                    height: 8
                                    radius: 4
                                    color: "#cba6f7"
                                }

                                Text {
                                    text: (delegateRoot.blockData.language && delegateRoot.blockData.language !== "")
                                          ? delegateRoot.blockData.language.toUpperCase()
                                          : "CODE"
                                    color: "#a6adc8"
                                    font.pixelSize: 11
                                    font.bold: true
                                    font.family: "monospace"
                                    font.letterSpacing: 0.8
                                    Layout.fillWidth: true
                                }

                                Rectangle {
                                    Layout.preferredWidth: copyBtnRow.implicitWidth + 18
                                    Layout.preferredHeight: 24
                                    radius: 6
                                    color: codeContainer.copied
                                           ? "#1e3a2f"
                                           : (copyMouse.containsMouse ? "#313244" : "#1e1e2e")
                                    border.color: codeContainer.copied ? "#a6e3a1" : "#45475a"
                                    border.width: 1
                                    Behavior on color { ColorAnimation { duration: 100 } }

                                    Row {
                                        id: copyBtnRow
                                        anchors.centerIn: parent
                                        spacing: 6

                                        Image {
                                            property string currentIcon: codeContainer.copied ? "check-tick" : "copy"
                                            property color currentColor: codeContainer.copied ? "#a6e3a1" : "#cdd6f4"

                                            width: 13
                                            height: 13
                                            sourceSize: Qt.size(13, 13)
                                            source: "image://icon/" + currentIcon + "?" + currentColor
                                            fillMode: Image.PreserveAspectFit
                                            smooth: true
                                            cache: false
                                            anchors.verticalCenter: parent.verticalCenter
                                        }

                                        Text {
                                            text: codeContainer.copied ? "Copiado" : "Copiar código"
                                            color: codeContainer.copied ? "#a6e3a1" : "#cdd6f4"
                                            font.pixelSize: 11
                                            font.bold: codeContainer.copied
                                            anchors.verticalCenter: parent.verticalCenter
                                        }
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

                        Item {
                            Layout.fillWidth: true
                            Layout.preferredHeight: codeTextEdit.contentHeight + 28

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
