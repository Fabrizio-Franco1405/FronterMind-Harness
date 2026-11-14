qt_add_library(FrontMarkdownViewer STATIC)

qt_add_qml_module(FrontMarkdownViewer
    URI FronterMind.Components.MarkdownViewer
    VERSION 1.0
    SOURCES
        MarkdownViewer.h
        MarkdownViewer.cpp
    QML_FILES
        MarkdownView.qml
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/FronterMind/Components/MarkdownViewer"
)

target_include_directories(FrontMarkdownViewer PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

if(WIN32)
    find_package(cmark-gfm CONFIG REQUIRED)
    find_package(cmark-gfm-extensions CONFIG REQUIRED)
    target_link_libraries(FrontMarkdownViewer
        PUBLIC
            Qt6::Quick
            Qt6::Gui
            Qt6::Qml
        PRIVATE
            libcmark-gfm
            libcmark-gfm-extensions
    )
else()
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
endif()
