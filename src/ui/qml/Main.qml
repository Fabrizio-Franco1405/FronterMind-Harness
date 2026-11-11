import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "components"
import "views"

ApplicationWindow {
    width: 1100
    height: 750
    visible: true
    title: "FronterMind - Harness"
    color: "#1e1e2e"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Sidebar {
                id: sidebar
                Layout.fillHeight: true
                currentIndex: stackLayout.currentIndex // <-- AÑADE ESTO
                onViewRequested: function(viewName) {
                    stackLayout.currentIndex = (viewName === "chat") ? 0 : 1;
                }
            }

            StackLayout {
                id: stackLayout
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: 0

                ChatView {}
                SettingsView {}
            }
        }

        StatusBar {
            Layout.fillWidth: true
        }
    }
}
