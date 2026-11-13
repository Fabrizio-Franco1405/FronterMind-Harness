import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: sidebar
    width: 220
    color: "#181825"

    signal viewRequested(string viewName)
    property int currentIndex: 0

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 10

        Text {
            text: "FronterMind"
            color: "#cba6f7"
            font.pixelSize: 20
            font.bold: true
            Layout.bottomMargin: 20
        }

        NavButton {
            text: "Chat"
            iconCode: "comment" // <-- Sin el -16
            isActive: sidebar.currentIndex === 0
            onClicked: sidebar.viewRequested("chat")
        }

        NavButton {
            text: "Ajustes"
            iconCode: "settings" // <-- Sin el -16
            isActive: sidebar.currentIndex === 1
            onClicked: sidebar.viewRequested("settings")
        }

        Item { Layout.fillHeight: true }

        Text {
            text: "v0.1.0 Alpha"
            color: "#6c7086"
            font.pixelSize: 12
        }
    }
}
