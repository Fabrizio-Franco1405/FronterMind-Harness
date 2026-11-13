import QtQuick

Item {
    id: root
    property string iconName: ""
    property color iconColor: "#cdd6f4"
    property int iconSize: 16

    width: iconSize
    height: iconSize

    Image {
        anchors.fill: parent
        // IMPORTANTE: sourceSize le dice al ImageProvider que renderice el SVG a este tamaño exacto
        sourceSize: Qt.size(root.iconSize, root.iconSize)
        source: root.iconName !== "" ? "image://icon/" + root.iconName + "?" + root.iconColor : ""
        fillMode: Image.PreserveAspectFit
        smooth: true
        // Evitar que Qt intente cachear la versión de 16x16
        cache: false
    }
}
