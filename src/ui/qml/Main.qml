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
    color: theme.bgBase

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
                currentIndex: stackLayout.currentIndex
                onViewRequested: function(viewName) {
                    if (viewName === "chat") stackLayout.currentIndex = 0;
                    else if (viewName === "settings") stackLayout.currentIndex = 1;
                    else if (viewName === "tools") stackLayout.currentIndex = 2;
                    else if (viewName === "appearance") stackLayout.currentIndex = 3;
                }
            }

            StackLayout {
                id: stackLayout
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: 0

                ChatView {}
                SettingsView {}
                ToolsMcpView {}
                AppearanceView {}
            }
        }

        StatusBar {
            Layout.fillWidth: true
        }
    }
}
