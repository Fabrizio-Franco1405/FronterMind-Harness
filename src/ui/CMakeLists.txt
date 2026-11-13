qt_add_library(FrontUI STATIC)

target_sources(FrontUI PRIVATE
    controllers/AppController.cpp
    controllers/AppController.h
    controllers/IconImageProvider.cpp
    controllers/IconImageProvider.h
)

qt_add_qml_module(FrontUI
    URI FronterMind.UI
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/components/Sidebar.qml
        qml/components/NavButton.qml
        qml/components/StatusBar.qml
        qml/components/ChatInput.qml
        qml/components/Icon.qml
        qml/views/ChatView.qml
        qml/views/SettingsView.qml
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/FronterMind/UI"
)

target_include_directories(FrontUI PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers
)

target_link_libraries(FrontUI
    PUBLIC
        Qt6::Quick
        FrontCore
        FrontMarkdownViewer
    PRIVATE
        Qt6::QmlPrivate
        Qt6::QuickDialogs2
        Qt6::Network
        qlementine-icons
        Qt6::Svg
)
