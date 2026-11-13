import QtQuick

Item {
    id: root
    property string markdownText: ""

    implicitWidth: textItem.implicitWidth
    implicitHeight: textItem.implicitHeight

    MarkdownParser {
        id: bridge
        markdown: root.markdownText
    }

    Text {
        id: textItem
        width: root.width
        textFormat: Text.RichText
        text: bridge.html
        wrapMode: Text.Wrap
        color: "#cdd6f4"
        lineHeight: 1.2
    }
}
