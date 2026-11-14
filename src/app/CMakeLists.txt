set(APP_RESOURCES "${CMAKE_SOURCE_DIR}/assets/resources.qrc")
if(WIN32 AND EXISTS "${CMAKE_SOURCE_DIR}/assets/icons/app_icon.rc")
    list(APPEND APP_RESOURCES "${CMAKE_SOURCE_DIR}/assets/icons/app_icon.rc")
endif()

qt_add_executable(FronterMind
    main.cpp
    ${APP_RESOURCES}
)

target_link_libraries(FronterMind PRIVATE
    FrontUI
    FrontUIplugin
    FrontMarkdownViewer
    FrontMarkdownViewerplugin
    qlementine-icons
    Qt6::Svg
)
