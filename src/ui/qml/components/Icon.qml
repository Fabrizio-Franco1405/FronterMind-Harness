import QtQuick

Text {
    property string iconCode: ""
    property color iconColor: "#cdd6f4"

    // Nombre de la fuente instalada en CachyOS
    font.family: "Font Awesome 6 Free"
    font.pixelSize: 14

    text: iconCode
    color: iconColor
    verticalAlignment: Text.AlignVCenter
    horizontalAlignment: Text.AlignHCenter
}
