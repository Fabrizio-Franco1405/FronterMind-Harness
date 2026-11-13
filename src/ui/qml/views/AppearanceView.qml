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

                RowLayout {
                    Layout.fillWidth: true
                    Layout.bottomMargin: 6

                    ColumnLayout {
                        spacing: 4
                        Text {
                            text: "Apariencia y Temas"
                            color: theme.textPrimary
                            font.family: theme.fontFamily
                            font.pixelSize: 26
                            font.bold: true
                        }
                        Text {
                            text: "Personaliza la paleta de colores, el contraste y las fuentes tipográficas en tiempo real."
                            color: theme.textSecondary
                            font.family: theme.fontFamily
                            font.pixelSize: 13
                        }
                    }

                    Item { Layout.fillWidth: true }

                    Button {
                        text: "Restaurar por defecto"
                        onClicked: theme.resetToDefault()
                        Layout.preferredHeight: 36
                        contentItem: Row {
                            spacing: 8
                            leftPadding: 14; rightPadding: 14
                            Icon { iconName: "reset"; iconColor: theme.textPrimary; iconSize: 14; anchors.verticalCenter: parent.verticalCenter }
                            Text {
                                text: "Restaurar por defecto"
                                color: theme.textPrimary
                                font.family: theme.fontFamily
                                font.pixelSize: 12
                                font.bold: true
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                        background: Rectangle {
                            color: parent.hovered ? theme.bgSurface : theme.bgSidebar
                            border.color: theme.borderColor
                            radius: 8
                        }
                    }
                }

                // --- SECCIÓN 1: PRESETS DE TEMAS ---
                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: presetCol.implicitHeight + 40
                    color: theme.bgSidebar
                    radius: 10
                    border.color: theme.borderColor
                    border.width: 1

                    ColumnLayout {
                        id: presetCol
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 14

                        Text {
                            text: "Temas Predefinidos"
                            color: theme.accentColor
                            font.family: theme.fontFamily
                            font.pixelSize: 15
                            font.bold: true
                        }

                        Flow {
                            Layout.fillWidth: true
                            spacing: 12

                            Repeater {
                                model: [
                                    { name: "Catppuccin Mocha", bg: "#1e1e2e", side: "#181825", acc: "#cba6f7", usr: "#89b4fa" },
                                    { name: "Tokyo Night",      bg: "#1a1b26", side: "#16161e", acc: "#7aa2f7", usr: "#bb9af7" },
                                    { name: "Rose Pine",        bg: "#1f1d2e", side: "#191724", acc: "#ebbcba", usr: "#c4a7e7" },
                                    { name: "Gruvbox Dark",     bg: "#282828", side: "#1d2021", acc: "#fe8019", usr: "#83a598" },
                                    { name: "Nord Polar",       bg: "#3b4252", side: "#2e3440", acc: "#88c0d0", usr: "#81a1c1" }
                                ]

                                delegate: Rectangle {
                                    width: 195
                                    height: 68
                                    radius: 8
                                    color: modelData.bg
                                    border.color: theme.currentPreset === modelData.name ? modelData.acc : theme.borderColor
                                    border.width: theme.currentPreset === modelData.name ? 2 : 1

                                    ColumnLayout {
                                        anchors.fill: parent
                                        anchors.margins: 12
                                        spacing: 8

                                        Text {
                                            text: modelData.name
                                            color: "#ffffff"
                                            font.family: theme.fontFamily
                                            font.pixelSize: 13
                                            font.bold: true
                                        }

                                        Row {
                                            spacing: 6
                                            Rectangle { width: 16; height: 16; radius: 8; color: modelData.side; border.color: "#555"; border.width: 1 }
                                            Rectangle { width: 16; height: 16; radius: 8; color: modelData.bg;   border.color: "#555"; border.width: 1 }
                                            Rectangle { width: 16; height: 16; radius: 8; color: modelData.acc }
                                            Rectangle { width: 16; height: 16; radius: 8; color: modelData.usr }
                                        }
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: theme.applyPreset(modelData.name)
                                    }
                                }
                            }
                        }
                    }
                }

                // --- SECCIÓN 2: TIPOGRAFÍA Y ESCALA ---
                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: fontCol.implicitHeight + 40
                    color: theme.bgSidebar
                    radius: 10
                    border.color: theme.borderColor
                    border.width: 1

                    ColumnLayout {
                        id: fontCol
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 14

                        Text {
                            text: "Tipografía y Tamaño de Letra"
                            color: theme.accentColor
                            font.family: theme.fontFamily
                            font.pixelSize: 15
                            font.bold: true
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 16

                            Text { text: "Fuente de la Interfaz:"; color: theme.textSecondary; font.family: theme.fontFamily; font.pixelSize: 13; Layout.preferredWidth: 180 }

                            ComboBox {
                                id: uiFontCombo
                                Layout.preferredWidth: 260
                                Layout.preferredHeight: 36
                                model: theme.systemFonts
                                Component.onCompleted: {
                                    var idx = theme.systemFonts.indexOf(theme.fontFamily)
                                    if (idx >= 0) currentIndex = idx
                                }
                                onActivated: theme.fontFamily = currentText
                                contentItem: Text {
                                    leftPadding: 12
                                    text: uiFontCombo.displayText
                                    color: theme.textPrimary
                                    font.family: uiFontCombo.displayText
                                    font.pixelSize: 13
                                    verticalAlignment: Text.AlignVCenter
                                }
                                background: Rectangle { color: theme.bgBase; border.color: theme.borderColor; radius: 6 }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 16

                            Text { text: "Fuente Monoespaciada (Código):"; color: theme.textSecondary; font.family: theme.fontFamily; font.pixelSize: 13; Layout.preferredWidth: 180 }

                            ComboBox {
                                id: monoFontCombo
                                Layout.preferredWidth: 260
                                Layout.preferredHeight: 36
                                model: theme.systemFonts
                                Component.onCompleted: {
                                    var idx = theme.systemFonts.indexOf(theme.monoFontFamily)
                                    if (idx >= 0) currentIndex = idx
                                }
                                onActivated: theme.monoFontFamily = currentText
                                contentItem: Text {
                                    leftPadding: 12
                                    text: monoFontCombo.displayText
                                    color: theme.textPrimary
                                    font.family: monoFontCombo.displayText
                                    font.pixelSize: 13
                                    verticalAlignment: Text.AlignVCenter
                                }
                                background: Rectangle { color: theme.bgBase; border.color: theme.borderColor; radius: 6 }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 16

                            Text { text: "Tamaño base de texto:"; color: theme.textSecondary; font.family: theme.fontFamily; font.pixelSize: 13; Layout.preferredWidth: 180 }

                            Slider {
                                id: sizeSlider
                                Layout.preferredWidth: 200
                                from: 11
                                to: 20
                                stepSize: 1
                                value: theme.baseFontSize
                                onMoved: theme.baseFontSize = Math.round(value)
                            }

                            Text {
                                text: theme.baseFontSize + " px"
                                color: theme.textPrimary
                                font.family: theme.monoFontFamily
                                font.pixelSize: 13
                                font.bold: true
                            }
                        }
                    }
                }

                // --- SECCIÓN 3: AJUSTE FINO DE COLORES (HEX) ---
                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: customColorCol.implicitHeight + 40
                    color: theme.bgSidebar
                    radius: 10
                    border.color: theme.borderColor
                    border.width: 1

                    ColumnLayout {
                        id: customColorCol
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 14

                        Text {
                            text: "Paleta de Colores Personalizada (HEX)"
                            color: theme.accentColor
                            font.family: theme.fontFamily
                            font.pixelSize: 15
                            font.bold: true
                        }

                        GridLayout {
                            Layout.fillWidth: true
                            columns: 2
                            rowSpacing: 12
                            columnSpacing: 24

                            Repeater {
                                model: [
                                    { label: "Color de Acento",       prop: "accentColor",     val: theme.accentColor },
                                    { label: "Burbuja del Usuario",   prop: "userBubbleColor", val: theme.userBubbleColor },
                                    { label: "Fondo Principal",       prop: "bgBase",          val: theme.bgBase },
                                    { label: "Fondo Barra Lateral",   prop: "bgSidebar",       val: theme.bgSidebar },
                                    { label: "Superficies y Tarjetas",prop: "bgSurface",       val: theme.bgSurface },
                                    { label: "Bordes y Divisores",    prop: "borderColor",     val: theme.borderColor },
                                    { label: "Texto Principal",       prop: "textPrimary",     val: theme.textPrimary },
                                    { label: "Texto Secundario",      prop: "textSecondary",   val: theme.textSecondary }
                                ]

                                delegate: RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 10

                                    Rectangle {
                                        width: 24; height: 24; radius: 6
                                        color: modelData.val
                                        border.color: theme.borderColor
                                        border.width: 1
                                    }

                                    Text {
                                        text: modelData.label
                                        color: theme.textSecondary
                                        font.family: theme.fontFamily
                                        font.pixelSize: 13
                                        Layout.preferredWidth: 155
                                    }

                                    TextField {
                                        Layout.preferredWidth: 110
                                        Layout.preferredHeight: 32
                                        text: modelData.val
                                        color: theme.textPrimary
                                        font.family: theme.monoFontFamily
                                        font.pixelSize: 12
                                        verticalAlignment: TextInput.AlignVCenter
                                        background: Rectangle { color: theme.bgBase; border.color: theme.borderColor; radius: 6 }
                                        onEditingFinished: {
                                            if (text.startsWith("#") && (text.length === 7 || text.length === 4)) {
                                                theme[modelData.prop] = text
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
}
