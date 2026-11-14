import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

Button {
    id: root
    property bool isActive: false
    property string iconCode: ""

    Layout.fillWidth: true
    Layout.preferredHeight: 42
    hoverEnabled: true

    background: Rectangle {
        color: root.isActive ? "#313244" : (root.hovered ? "#242436" : "transparent")
        radius: 8
        border.color: root.isActive ? "#45475a" : "transparent"
        border.width: 1
        Behavior on color { ColorAnimation { duration: 140 } }

        // Barra vertical de acento cuando está activo
        Rectangle {
            visible: root.isActive
            width: 3
            height: 18
            radius: 2
            color: "#cba6f7"
            anchors.left: parent.left
            anchors.leftMargin: 4
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    contentItem: RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 12
        spacing: 12

        Icon {
            iconName: root.iconCode
            iconColor: root.isActive ? "#cba6f7" : (root.hovered ? "#cdd6f4" : "#a6adc8")
            iconSize: 16
        }

        Text {
            text: root.text
            color: root.isActive ? "#cdd6f4" : (root.hovered ? "#cdd6f4" : "#a6adc8")
            font.pixelSize: 13
            font.bold: root.isActive
            Layout.fillWidth: true
            verticalAlignment: Text.AlignVCenter
        }
    }
}
