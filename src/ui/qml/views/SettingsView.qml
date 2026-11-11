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

                Text {
                    text: "Configuración del Motor"
                    color: "#cdd6f4"
                    font.pixelSize: 26
                    font.bold: true
                    Layout.bottomMargin: 10
                }

                // --- SECCIÓN 1: MOTOR Y RUTA ---
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 150 // Un poco más alto para que respire
                    color: "#181825"
                    radius: 8
                    border.color: "#313244"

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 15

                        Text { text: "Backend de IA Local"; color: "#cba6f7"; font.pixelSize: 16; font.bold: true }

                        RowLayout {
                            Layout.fillWidth: true
                            Text { text: "Motor:"; color: "#a6adc8"; font.pixelSize: 14; Layout.preferredWidth: 80 }
                            ComboBox {
                                id: engineCombo
                                Layout.preferredWidth: 200
                                Layout.preferredHeight: 36 // Altura corregida
                                model: backend.availableEngines
                                currentIndex: backend.availableEngines.indexOf(backend.currentEngine)
                                onActivated: backend.currentEngine = currentText
                                contentItem: Text {
                                    leftPadding: 10
                                    text: engineCombo.displayText
                                    color: "#cdd6f4"
                                    verticalAlignment: Text.AlignVCenter
                                }
                                background: Rectangle {
                                    color: "#1e1e2e"
                                    border.color: "#45475a"
                                    radius: 4
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Text { text: "Ruta del binario / API:"; color: "#a6adc8"; font.pixelSize: 14; Layout.preferredWidth: 150 }
                            TextField {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 36 // Altura corregida
                                text: backend.enginePath
                                color: "#cdd6f4"
                                verticalAlignment: TextInput.AlignVCenter // Centrar texto
                                background: Rectangle { color: "#1e1e2e"; border.color: "#45475a"; radius: 4 }
                                onEditingFinished: backend.enginePath = text
                                placeholderText: "Ej. /usr/bin/llama-server o http://localhost:8080"
                            }
                        }
                    }
                }

                // --- SECCIÓN 2: MODELOS ---
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 250
                    color: "#181825"
                    radius: 8
                    border.color: "#313244"

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 15

                        RowLayout {
                            Layout.fillWidth: true
                            Text { text: "Modelos Locales"; color: "#cba6f7"; font.pixelSize: 16; font.bold: true }
                            Item { Layout.fillWidth: true }
                            Button {
                                text: "+ Añadir Modelo"
                                onClicked: addModelDialog.open()
                                contentItem: Text { text: parent.text; color: "#1e1e2e"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                background: Rectangle { color: parent.hovered ? "#b4befe" : "#89b4fa"; radius: 4; implicitWidth: 120; implicitHeight: 30 }
                            }
                        }

                        ListView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true
                            spacing: 10
                            model: backend.localModels

                            delegate: Rectangle {
                                width: ListView.view.width
                                height: 60
                                color: "#1e1e2e"
                                radius: 6
                                border.color: backend.currentModel.startsWith(modelData.alias) ? "#a6e3a1" : "#313244"
                                border.width: backend.currentModel.startsWith(modelData.alias) ? 2 : 1

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2
                                        Text { text: modelData.alias; color: "#cdd6f4"; font.pixelSize: 16; font.bold: true }
                                        Text { text: modelData.path; color: "#6c7086"; font.pixelSize: 11; elide: Text.ElideMiddle; Layout.fillWidth: true }
                                    }
                                    Button {
                                        text: backend.currentModel.startsWith(modelData.alias) ? "Cargado" : "Cargar"
                                        enabled: !backend.currentModel.startsWith(modelData.alias)
                                        Layout.preferredWidth: 80
                                        Layout.preferredHeight: 30
                                        onClicked: backend.loadModel(modelData.alias, modelData.path)
                                        contentItem: Text { text: parent.text; color: "#1e1e2e"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                        background: Rectangle { color: parent.enabled ? "#a6e3a1" : "#585b70"; radius: 4 }
                                    }
                                }
                            }
                            Text {
                                anchors.centerIn: parent
                                text: "No hay modelos añadidos."
                                color: "#6c7086"
                                visible: backend.localModels.length === 0
                            }
                        }
                    }
                }

                // --- SECCIÓN 3: CONTROLES DEL MOTOR ---
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 80
                    color: "#181825"
                    radius: 8
                    border.color: "#313244"

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 20

                        // Botón de Iniciar
                        Button {
                            id: startBtn
                            enabled: !backend.engineManager.isRunning && backend.currentModel !== "Ninguno"
                            Layout.preferredWidth: 200
                            Layout.preferredHeight: 40

                            background: Rectangle {
                                color: startBtn.enabled ? "#a6e3a1" : (backend.engineManager.isRunning ? "#f9e2af" : "#585b70")
                                radius: 6
                                border.color: Qt.darker(color, 1.2)
                                border.width: 1
                            }

                            // Contenido centrado usando anchors
                            contentItem: Item {
                                Row {
                                    anchors.centerIn: parent
                                    spacing: 8
                                    Icon { iconCode: "\uf04b"; iconColor: "#1e1e2e"; font.pixelSize: 14 }
                                    Text {
                                        text: backend.engineManager.isRunning ? "Motor en Ejecución" : "Iniciar Motor"
                                        color: "#1e1e2e"
                                        font.bold: true
                                        font.pixelSize: 14
                                    }
                                }
                            }

                            onClicked: {
                                if (backend.localModels.length > 0) {
                                    var currentAlias = backend.currentModel.split(" (")[0];
                                    for (var i = 0; i < backend.localModels.length; i++) {
                                        if (backend.localModels[i].alias === currentAlias) {
                                            backend.engineManager.startServer(backend.enginePath, backend.localModels[i].path, 99);
                                            break;
                                        }
                                    }
                                }
                            }
                        }

                        // Botón de Detener
                        Button {
                            id: stopBtn
                            enabled: backend.engineManager.isRunning
                            Layout.preferredWidth: 160
                            Layout.preferredHeight: 40

                            background: Rectangle {
                                color: stopBtn.enabled ? "#f38ba8" : "#585b70"
                                radius: 6
                                border.color: Qt.darker(color, 1.2)
                                border.width: 1
                            }

                            contentItem: Item {
                                Row {
                                    anchors.centerIn: parent
                                    spacing: 8
                                    Icon { iconCode: "\uf04d"; iconColor: "#1e1e2e"; font.pixelSize: 14 }
                                    Text {
                                        text: "Detener Motor"
                                        color: "#1e1e2e"
                                        font.bold: true
                                        font.pixelSize: 14
                                    }
                                }
                            }

                            onClicked: backend.engineManager.stopServer()
                        }
                    }
                }

                // --- SECCIÓN 4: CONSOLA DE LOGS ---
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 180
                    color: "#11111b"
                    radius: 8
                    border.color: "#45475a"
                    clip: true

                    ScrollView {
                        anchors.fill: parent
                        anchors.margins: 10
                        TextArea {
                            text: backend.engineManager.serverLog
                            color: "#a6adc8"
                            font.family: "monospace"
                            font.pixelSize: 12
                            readOnly: true
                            wrapMode: TextArea.Wrap
                            background: null
                        }
                    }
                }
            }
        }
    }

    // --- MODAL PARA AÑADIR MODELO Y ALIAS ---
    Dialog {
        id: addModelDialog
        title: "Añadir Modelo Local"
        modal: true
        anchors.centerIn: Overlay.overlay
        width: 500
        background: Rectangle { color: "#1e1e2e"; radius: 8; border.color: "#45475a" }

        ColumnLayout {
            anchors.fill: parent
            spacing: 15

            Text { text: "Ruta del archivo (.gguf):"; color: "#cdd6f4" }
            RowLayout {
                Layout.fillWidth: true
                TextField {
                    id: pathInput
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
                    placeholderText: "Pega la ruta aquí..."
                    color: "#cdd6f4"
                    verticalAlignment: TextInput.AlignVCenter
                    background: Rectangle { color: "#181825"; border.color: "#45475a"; radius: 4 }
                }
                Button {
                    text: "Buscar..."
                    Layout.preferredHeight: 36
                    onClicked: fileDialog.open()
                    background: Rectangle { color: "#45475a"; radius: 4 }
                    contentItem: Text { text: parent.text; color: "#cdd6f4"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                }
            }

            Text { text: "Alias (Nombre visible):"; color: "#cdd6f4"; Layout.topMargin: 10 }
            TextField {
                id: aliasInput
                Layout.fillWidth: true
                Layout.preferredHeight: 36
                placeholderText: "Ej. Qwen 2.5 Coder"
                color: "#cdd6f4"
                verticalAlignment: TextInput.AlignVCenter
                background: Rectangle { color: "#181825"; border.color: "#45475a"; radius: 4 }
            }

            // Botones del Modal centrados
            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 20
                spacing: 10

                Item { Layout.fillWidth: true } // Espaciador

                Button {
                    text: "Cancelar"
                    Layout.preferredWidth: 100
                    Layout.preferredHeight: 35
                    onClicked: addModelDialog.close()
                    background: Rectangle { color: "#45475a"; radius: 4 }
                    contentItem: Text { text: parent.text; color: "#cdd6f4"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                }
                Button {
                    text: "Guardar"
                    Layout.preferredWidth: 100
                    Layout.preferredHeight: 35
                    enabled: pathInput.text !== "" && aliasInput.text !== ""
                    onClicked: {
                        backend.addModel(pathInput.text, aliasInput.text)
                        pathInput.text = ""
                        aliasInput.text = ""
                        addModelDialog.close()
                    }
                    background: Rectangle { color: parent.enabled ? "#a6e3a1" : "#585b70"; radius: 4 }
                    contentItem: Text { text: parent.text; color: "#1e1e2e"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                }
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: "Selecciona un modelo GGUF"
        nameFilters: ["Modelos GGUF (*.gguf)", "Todos los archivos (*)"]
        onAccepted: pathInput.text = selectedFile.toString().replace("file://", "")
    }
}
