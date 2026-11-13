qt_add_library(FrontMarkdownViewer STATIC)

qt_add_qml_module(FrontMarkdownViewer
    URI FronterMind.Components.MarkdownViewer
    VERSION 1.0
    SOURCES
        include/MarkdownViewer.h
        src/MarkdownViewer.cpp
    QML_FILES
        MarkdownView.qml
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/FronterMind/Components/MarkdownViewer"
)

target_include_directories(FrontMarkdownViewer PUBLIC include)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CMARK_GFM REQUIRED IMPORTED_TARGET libcmark-gfm)

target_link_libraries(FrontMarkdownViewer
    PUBLIC
        Qt6::Quick
        Qt6::Gui
        Qt6::Qml
    PRIVATE
        PkgConfig::CMARK_GFM
        cmark-gfm-extensions
)
