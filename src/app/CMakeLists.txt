qt_add_executable(FronterMind main.cpp)

target_link_libraries(FronterMind PRIVATE
    FrontUI
    FrontUIplugin
    FrontMarkdownViewer
    FrontMarkdownViewerplugin
    qlementine-icons
    Qt6::Svg
)
