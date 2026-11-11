import QtQuick

Text {
    property string iconCode: ""
    property color iconColor: "#cdd6f4"

    font.family: "Font Awesome 6 Free"
    font.weight: Font.Bold // <-- CRUCIAL: Obliga a usar la versión "Solid"
    font.pixelSize: 14

    text: iconCode
    color: iconColor
    verticalAlignment: Text.AlignVCenter
    horizontalAlignment: Text.AlignHCenter
}
