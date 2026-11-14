import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
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

                // --- CABECERA Y CONTROLES RÁPIDOS DEL SERVIDOR ---
                RowLayout {
                    Layout.fillWidth: true
                    Layout.bottomMargin: 4
                    spacing: 12

                    ColumnLayout {
                        spacing: 4
                        Text {
                            text: "Motor de Inferencia & Modelos"
                            color: theme.textPrimary
                            font.family: theme.fontFamily
                            font.pixelSize: 26
                            font.bold: true
                        }
                        Text {
                            text: "Administra el servidor local, el presupuesto de contexto y tu biblioteca de modelos GGUF."
                            color: theme.textSecondary
                            font.family: theme.fontFamily
                            font.pixelSize: 13
                        }
                    }

                    Item { Layout.fillWidth: true }

                    // Botón Iniciar Motor (100% centrado con padding: 0)
                    Button {
                        id: startBtn
                        padding: 0
                        topPadding: 0; bottomPadding: 0; leftPadding: 0; rightPadding: 0
                        enabled: !backend.engineManager.isRunning && backend.currentModel !== "Ninguno"
                        Layout.preferredHeight: 40
                        Layout.preferredWidth: 165

                        background: Rectangle {
                            color: startBtn.enabled
                                   ? (startBtn.hovered ? "#b8f0b3" : "#a6e3a1")
                                   : (backend.engineManager.isRunning ? "#1e3a2f" : theme.bgSurface)
                            radius: 8
                            border.color: backend.engineManager.isRunning ? "#a6e3a1" : "transparent"
                            border.width: backend.engineManager.isRunning ? 1 : 0
                            Behavior on color { ColorAnimation { duration: 120 } }
                        }

                        contentItem: Item {
                            Row {
                                anchors.centerIn: parent
                                spacing: 8
                                Icon {
                                    iconName: "play"
                                    iconColor: startBtn.enabled ? "#11111b" : (backend.engineManager.isRunning ? "#a6e3a1" : "#6c7086")
                                    iconSize: 14
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                                Text {
                                    text: backend.engineManager.isRunning ? "Motor Activo" : "Iniciar Motor"
                                    color: startBtn.enabled ? "#11111b" : (backend.engineManager.isRunning ? "#a6e3a1" : "#6c7086")
                                    font.family: theme.fontFamily
                                    font.bold: true
                                    font.pixelSize: 13
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }
                        }

                        onClicked: {
                            if (backend.localModels.length > 0) {
                                var currentAlias = backend.currentModel.split(" (")[0]
                                for (var i = 0; i < backend.localModels.length; i++) {
                                    if (backend.localModels[i].alias === currentAlias) {
                                        backend.engineManager.startServer(backend.enginePath, backend.localModels[i].path, 99, backend.maxContext)
                                        break
                                    }
                                }
                            }
                        }
                    }

                    // Botón Detener Motor (100% centrado con padding: 0)
                    Button {
                        id: stopBtn
                        padding: 0
                        topPadding: 0; bottomPadding: 0; leftPadding: 0; rightPadding: 0
                        enabled: backend.engineManager.isRunning
                        Layout.preferredHeight: 40
                        Layout.preferredWidth: 155

                        background: Rectangle {
                            color: stopBtn.enabled ? (stopBtn.hovered ? "#f7a1b8" : "#f38ba8") : theme.bgSurface
                            radius: 8
                            Behavior on color { ColorAnimation { duration: 120 } }
                        }

                        contentItem: Item {
                            Row {
                                anchors.centerIn: parent
                                spacing: 8
                                Icon {
                                    iconName: "stop"
                                    iconColor: stopBtn.enabled ? "#11111b" : "#6c7086"
                                    iconSize: 14
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                                Text {
                                    text: "Detener Motor"
                                    color: stopBtn.enabled ? "#11111b" : "#6c7086"
                                    font.family: theme.fontFamily
                                    font.bold: true
                                    font.pixelSize: 13
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }
                        }

                        onClicked: backend.engineManager.stopServer()
                    }
                }

                // --- SECCIÓN 1: PARÁMETROS DEL BACKEND ---
                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: backendCol.implicitHeight + 40
                    color: theme.bgSidebar
                    radius: 10
                    border.color: theme.borderColor
                    border.width: 1

                    ColumnLayout {
                        id: backendCol
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 14

                        RowLayout {
                            spacing: 8
                            Icon { iconName: "settings"; iconColor: theme.accentColor; iconSize: 15 }
                            Text {
                                text: "Configuración del Backend Local"
                                color: theme.accentColor
                                font.family: theme.fontFamily
                                font.pixelSize: 15
                                font.bold: true
                            }
                        }

                        GridLayout {
                            Layout.fillWidth: true
                            columns: 2
                            rowSpacing: 14
                            columnSpacing: 20

                            Text {
                                text: "Motor de Inferencia:"
                                color: theme.textSecondary
                                font.family: theme.fontFamily
                                font.pixelSize: 13
                                Layout.preferredWidth: 170
                            }

                            ComboBox {
                                id: engineCombo
                                Layout.preferredWidth: 240
                                Layout.preferredHeight: 36
                                model: backend.availableEngines
                                currentIndex: backend.availableEngines.indexOf(backend.currentEngine)
                                onActivated: backend.currentEngine = currentText
                                contentItem: Text {
                                    leftPadding: 12
                                    text: engineCombo.displayText
                                    color: theme.textPrimary
                                    font.family: theme.fontFamily
                                    font.pixelSize: 13
                                    verticalAlignment: Text.AlignVCenter
                                }
                                background: Rectangle { color: theme.bgBase; border.color: theme.borderColor; radius: 6 }
                            }

                            Text {
                                text: "Ventana de Contexto:"
                                color: theme.textSecondary
                                font.family: theme.fontFamily
                                font.pixelSize: 13
                            }

                            RowLayout {
                                spacing: 10
                                TextField {
                                    Layout.preferredWidth: 140
                                    Layout.preferredHeight: 36
                                    text: backend.maxContext
                                    color: theme.textPrimary
                                    font.family: theme.monoFontFamily
                                    font.pixelSize: 13
                                    verticalAlignment: TextInput.AlignVCenter
                                    background: Rectangle { color: theme.bgBase; border.color: theme.borderColor; radius: 6 }
                                    validator: IntValidator { bottom: 512; top: 131072 }
                                    onEditingFinished: backend.maxContext = parseInt(text)
                                }

                                Repeater {
                                    model: [4096, 8192, 16384, 32768]
                                    delegate: Rectangle {
                                        width: 56; height: 28; radius: 6
                                        color: backend.maxContext === modelData ? theme.accentColor : theme.bgBase
                                        border.color: theme.borderColor
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: (modelData / 1024) + "K"
                                            color: backend.maxContext === modelData ? "#11111b" : theme.textSecondary
                                            font.family: theme.monoFontFamily
                                            font.pixelSize: 11
                                            font.bold: true
                                        }
                                        MouseArea {
                                            anchors.fill: parent
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: backend.maxContext = modelData
                                        }
                                    }
                                }
                            }

                            Text {
                                text: "Ruta del Binario / API:"
                                color: theme.textSecondary
                                font.family: theme.fontFamily
                                font.pixelSize: 13
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 8
                                TextField {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 36
                                    text: backend.enginePath
                                    color: theme.textPrimary
                                    font.family: theme.monoFontFamily
                                    font.pixelSize: 12
                                    verticalAlignment: TextInput.AlignVCenter
                                    background: Rectangle { color: theme.bgBase; border.color: theme.borderColor; radius: 6 }
                                    onEditingFinished: backend.enginePath = text
                                    placeholderText: "/usr/bin/llama-server"
                                }
                                Button {
                                    padding: 0
                                    Layout.preferredHeight: 36
                                    Layout.preferredWidth: 40
                                    onClicked: binaryDialog.open()
                                    background: Rectangle { color: parent.hovered ? theme.bgSurface : theme.bgBase; border.color: theme.borderColor; radius: 6 }
                                    contentItem: Item {
                                        Icon { anchors.centerIn: parent; iconName: "folder-open"; iconColor: theme.textPrimary; iconSize: 15 }
                                    }
                                }
                            }
                        }
                    }
                }

                // --- SECCIÓN 2: BIBLIOTECA DE MODELOS LOCALES ---
                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: modelsCol.implicitHeight + 40
                    color: theme.bgSidebar
                    radius: 10
                    border.color: theme.borderColor
                    border.width: 1

                    ColumnLayout {
                        id: modelsCol
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 14

                        RowLayout {
                            Layout.fillWidth: true
                            Icon { iconName: "database"; iconColor: theme.accentColor; iconSize: 15 }
                            Text {
                                text: "Modelos Locales GGUF (" + backend.localModels.length + ")"
                                color: theme.accentColor
                                font.family: theme.fontFamily
                                font.pixelSize: 15
                                font.bold: true
                            }
                            Item { Layout.fillWidth: true }

                            Button {
                                padding: 0
                                topPadding: 0; bottomPadding: 0; leftPadding: 0; rightPadding: 0
                                onClicked: addModelDialog.open()
                                Layout.preferredHeight: 34
                                Layout.preferredWidth: 145
                                background: Rectangle {
                                    color: parent.hovered ? "#b4befe" : theme.userBubbleColor
                                    radius: 6
                                }
                                contentItem: Item {
                                    Row {
                                        anchors.centerIn: parent
                                        spacing: 6
                                        Icon { iconName: "plus"; iconColor: "#11111b"; iconSize: 13; anchors.verticalCenter: parent.verticalCenter }
                                        Text {
                                            text: "Añadir Modelo"
                                            color: "#11111b"
                                            font.family: theme.fontFamily
                                            font.pixelSize: 12
                                            font.bold: true
                                            anchors.verticalCenter: parent.verticalCenter
                                        }
                                    }
                                }
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Repeater {
                                model: backend.localModels

                                delegate: Rectangle {
                                    readonly property bool isSelected: backend.currentModel.startsWith(modelData.alias + " (")
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 62
                                    color: theme.bgBase
                                    radius: 8
                                    border.color: isSelected ? "#a6e3a1" : theme.borderColor
                                    border.width: isSelected ? 1.5 : 1

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.leftMargin: 14
                                        anchors.rightMargin: 12
                                        spacing: 12

                                        Rectangle {
                                            width: 34; height: 34; radius: 8
                                            color: isSelected ? "#1e3a2f" : theme.bgSidebar
                                            border.color: isSelected ? "#a6e3a1" : theme.borderColor
                                            Icon {
                                                anchors.centerIn: parent
                                                iconName: "file"
                                                iconColor: isSelected ? "#a6e3a1" : theme.textSecondary
                                                iconSize: 15
                                            }
                                        }

                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 2
                                            Text {
                                                text: modelData.alias
                                                color: theme.textPrimary
                                                font.family: theme.fontFamily
                                                font.pixelSize: 14
                                                font.bold: true
                                            }
                                            Text {
                                                text: modelData.path
                                                color: theme.textSecondary
                                                font.family: theme.monoFontFamily
                                                font.pixelSize: 11
                                                elide: Text.ElideMiddle
                                                Layout.fillWidth: true
                                            }
                                        }

                                        Button {
                                            padding: 0
                                            text: isSelected ? "Seleccionado" : "Seleccionar"
                                            enabled: !isSelected
                                            Layout.preferredWidth: 105
                                            Layout.preferredHeight: 32
                                            onClicked: backend.loadModel(modelData.alias, modelData.path)
                                            contentItem: Text {
                                                text: parent.text
                                                color: isSelected ? "#a6e3a1" : "#11111b"
                                                font.family: theme.fontFamily
                                                font.pixelSize: 12
                                                font.bold: true
                                                horizontalAlignment: Text.AlignHCenter
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                            background: Rectangle {
                                                color: isSelected ? "#1e3a2f" : "#a6e3a1"
                                                border.color: isSelected ? "#a6e3a1" : "transparent"
                                                radius: 6
                                            }
                                        }

                                        Rectangle {
                                            Layout.preferredWidth: 32
                                            Layout.preferredHeight: 32
                                            radius: 6
                                            color: delModelMouse.containsMouse ? theme.bgSurface : "transparent"
                                            border.color: delModelMouse.containsMouse ? "#f38ba8" : theme.borderColor
                                            border.width: 1

                                            Icon {
                                                anchors.centerIn: parent
                                                iconName: "trash"
                                                iconColor: "#f38ba8"
                                                iconSize: 14
                                            }

                                            MouseArea {
                                                id: delModelMouse
                                                anchors.fill: parent
                                                hoverEnabled: true
                                                cursorShape: Qt.PointingHandCursor
                                                onClicked: backend.removeModel(index)
                                            }
                                        }
                                    }
                                }
                            }

                            Text {
                                Layout.alignment: Qt.AlignHCenter
                                Layout.topMargin: 12
                                Layout.bottomMargin: 12
                                text: "No hay modelos registrados. Añade un archivo .gguf para comenzar."
                                color: theme.textSecondary
                                font.family: theme.fontFamily
                                font.pixelSize: 13
                                visible: backend.localModels.length === 0
                            }
                        }
                    }
                }

                // --- SECCIÓN 3: CONSOLA DE PROCESOS EN TIEMPO REAL ---
                Rectangle {
                    id: consoleCard
                    Layout.fillWidth: true
                    Layout.preferredHeight: 240
                    color: theme.bgDarkest
                    radius: 10
                    border.color: theme.borderColor
                    border.width: 1
                    clip: true

                    property bool logsCopied: false

                    Timer {
                        id: logCopyTimer
                        interval: 1800
                        onTriggered: consoleCard.logsCopied = false
                    }

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 0

                        // Barra superior de la consola con botón Copiar Logs
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 38
                            color: theme.bgSidebar

                            Rectangle {
                                width: parent.width; height: 1
                                color: theme.borderColor
                                anchors.bottom: parent.bottom
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 14
                                anchors.rightMargin: 12
                                spacing: 10

                                Rectangle {
                                    width: 8; height: 8; radius: 4
                                    color: backend.engineManager.isRunning ? "#a6e3a1" : "#6c7086"
                                }

                                Text {
                                    text: "CONSOLA DEL SERVIDOR EN TIEMPO REAL (STDOUT / STDERR)"
                                    color: theme.textSecondary
                                    font.family: theme.monoFontFamily
                                    font.pixelSize: 11
                                    font.bold: true
                                    Layout.fillWidth: true
                                }

                                // Botón Copiar Logs de la Consola
                                Rectangle {
                                    Layout.preferredWidth: copyLogRow.implicitWidth + 18
                                    Layout.preferredHeight: 24
                                    radius: 6
                                    color: consoleCard.logsCopied
                                           ? "#1e3a2f"
                                           : (copyLogMouse.containsMouse ? theme.bgSurface : theme.bgBase)
                                    border.color: consoleCard.logsCopied ? "#a6e3a1" : theme.borderColor
                                    border.width: 1

                                    Row {
                                        id: copyLogRow
                                        anchors.centerIn: parent
                                        spacing: 6

                                        Icon {
                                            iconName: consoleCard.logsCopied ? "check-tick" : "copy"
                                            iconColor: consoleCard.logsCopied ? "#a6e3a1" : theme.textPrimary
                                            iconSize: 12
                                            anchors.verticalCenter: parent.verticalCenter
                                        }

                                        Text {
                                            text: consoleCard.logsCopied ? "Copiado" : "Copiar consola"
                                            color: consoleCard.logsCopied ? "#a6e3a1" : theme.textPrimary
                                            font.family: theme.fontFamily
                                            font.pixelSize: 11
                                            font.bold: consoleCard.logsCopied
                                            anchors.verticalCenter: parent.verticalCenter
                                        }
                                    }

                                    MouseArea {
                                        id: copyLogMouse
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: {
                                            logArea.selectAll()
                                            logArea.copy()
                                            logArea.deselect()
                                            consoleCard.logsCopied = true
                                            logCopyTimer.restart()
                                        }
                                    }
                                }

                                Text {
                                    text: backend.engineManager.isRunning ? "ONLINE" : "OFFLINE"
                                    color: backend.engineManager.isRunning ? "#a6e3a1" : "#f38ba8"
                                    font.family: theme.monoFontFamily
                                    font.pixelSize: 10
                                    font.bold: true
                                }
                            }
                        }

                        ScrollView {
                            id: logScroll
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            anchors.margins: 12
                            clip: true

                            TextArea {
                                id: logArea
                                text: backend.engineManager.serverLog !== ""
                                      ? backend.engineManager.serverLog
                                      : "[Sistema] Esperando inicio del motor llama-server..."
                                color: theme.textSecondary
                                font.family: theme.monoFontFamily
                                font.pixelSize: 12
                                readOnly: true
                                selectByMouse: true
                                wrapMode: TextArea.Wrap
                                background: null
                                onTextChanged: {
                                    cursorPosition = length
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // --- MODAL PARA AÑADIR MODELO Y ALIAS ---
    Dialog {
        id: addModelDialog
        title: "Registrar Modelo GGUF"
        modal: true
        anchors.centerIn: Overlay.overlay
        width: 520
        background: Rectangle { color: theme.bgBase; radius: 10; border.color: theme.borderColor; border.width: 1 }

        ColumnLayout {
            anchors.fill: parent
            spacing: 14

            Text { text: "Ruta del archivo (.gguf):"; color: theme.textPrimary; font.family: theme.fontFamily; font.pixelSize: 13 }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                TextField {
                    id: pathInput
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
                    placeholderText: "/ruta/al/modelo.gguf"
                    color: theme.textPrimary
                    font.family: theme.monoFontFamily
                    font.pixelSize: 12
                    verticalAlignment: TextInput.AlignVCenter
                    background: Rectangle { color: theme.bgSidebar; border.color: theme.borderColor; radius: 6 }
                    onTextChanged: {
                        if (aliasInput.text === "" && text.endsWith(".gguf")) {
                            var parts = text.replace(/\\/g, "/").split("/")
                            aliasInput.text = parts[parts.length - 1]
                        }
                    }
                }
                Button {
                    padding: 0
                    text: "Explorar"
                    Layout.preferredWidth: 85
                    Layout.preferredHeight: 36
                    onClicked: fileDialog.open()
                    background: Rectangle { color: theme.bgSurface; border.color: theme.borderColor; radius: 6 }
                    contentItem: Text { text: parent.text; color: theme.textPrimary; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                }
            }

            Text { text: "Alias (Nombre visible):"; color: theme.textPrimary; font.family: theme.fontFamily; font.pixelSize: 13 }

            TextField {
                id: aliasInput
                Layout.fillWidth: true
                Layout.preferredHeight: 36
                placeholderText: "Ej. Qwen3-8B-Q4_K_M"
                color: theme.textPrimary
                font.family: theme.fontFamily
                font.pixelSize: 13
                verticalAlignment: TextInput.AlignVCenter
                background: Rectangle { color: theme.bgSidebar; border.color: theme.borderColor; radius: 6 }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 14
                spacing: 10

                Item { Layout.fillWidth: true }

                Button {
                    padding: 0
                    text: "Cancelar"
                    Layout.preferredWidth: 105
                    Layout.preferredHeight: 36
                    onClicked: addModelDialog.close()
                    background: Rectangle { color: theme.bgSurface; radius: 6 }
                    contentItem: Text { text: parent.text; color: theme.textPrimary; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                }
                Button {
                    padding: 0
                    text: "Guardar Modelo"
                    Layout.preferredWidth: 130
                    Layout.preferredHeight: 36
                    enabled: pathInput.text.trim() !== "" && aliasInput.text.trim() !== ""
                    onClicked: {
                        backend.addModel(pathInput.text.trim(), aliasInput.text.trim())
                        pathInput.text = ""
                        aliasInput.text = ""
                        addModelDialog.close()
                    }
                    background: Rectangle { color: parent.enabled ? "#a6e3a1" : theme.bgSurface; radius: 6 }
                    contentItem: Text { text: parent.text; color: "#11111b"; font.bold: true; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                }
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: "Selecciona un modelo GGUF"
        nameFilters: ["Modelos GGUF (*.gguf)", "Todos los archivos (*)"]
        onAccepted: pathInput.text = selectedFile.toString().replace("file:///", "/").replace("file://", "")
    }

    FileDialog {
        id: binaryDialog
        title: "Selecciona el binario de llama-server"
        onAccepted: backend.enginePath = selectedFile.toString().replace("file:///", "/").replace("file://", "")
    }
}
