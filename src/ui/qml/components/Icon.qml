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
        sourceSize: Qt.size(root.iconSize, root.iconSize)
        source: root.iconName !== "" ? "image://icon/" + root.iconName + "?" + root.iconColor : ""
        fillMode: Image.PreserveAspectFit
        smooth: true
        cache: true
    }
}
