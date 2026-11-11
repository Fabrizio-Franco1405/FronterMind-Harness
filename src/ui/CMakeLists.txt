qt_standard_project_setup(REQUIRES 6.5)
qt_policy(SET QTP0001 NEW)

qt_add_library(FrontUI STATIC)

# Añadimos el controlador
target_sources(FrontUI PRIVATE
    src/AppController.cpp
    include/AppController.h
)

# El módulo QML debe incluir Main.qml
qt_add_qml_module(FrontUI
    URI FronterMind.UI
    VERSION 1.0
    QML_FILES
        qml/Main.qml
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/FronterMind/UI"
)

target_include_directories(FrontUI PUBLIC include)

target_link_libraries(FrontUI
    PUBLIC Qt6::Quick
    PRIVATE FrontCore Qt6::QmlPrivate
)
