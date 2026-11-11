import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

Button {
    id: root
    property bool isActive: false
    property string iconCode: ""

    Layout.fillWidth: true
    Layout.preferredHeight: 40

    background: Rectangle {
        color: root.isActive ? "#45475a" : "transparent"
        radius: 6
        opacity: root.hovered || root.isActive ? 1.0 : 0.5
        Behavior on opacity { NumberAnimation { duration: 150 } }
    }

    contentItem: Row {
        anchors.left: parent.left
        anchors.leftMargin: 15
        anchors.verticalCenter: parent.verticalCenter
        spacing: 12

        Icon {
            iconCode: root.iconCode
            iconColor: root.isActive ? "#cba6f7" : "#cdd6f4"
            font.pixelSize: 16
        }

        Text {
            text: root.text
            color: "#cdd6f4"
            font.pixelSize: 14
            font.bold: root.isActive
            verticalAlignment: Text.AlignVCenter
        }
    }
}
