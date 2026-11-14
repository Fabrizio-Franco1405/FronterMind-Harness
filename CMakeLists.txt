cmake_minimum_required(VERSION 3.28)

if(UNIX AND NOT APPLE)
    set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "f35a9ac6-8463-4d38-8eec-5d6008153e7d")
endif()

project(FronterMind-Harness VERSION 0.1.0 LANGUAGES CXX)

# 1. Cargar librerías externas ANTES de activar CMAKE_CXX_MODULE_STD
include(FetchContent)
FetchContent_Declare(
    qlementine-icons
    GIT_REPOSITORY "https://github.com/oclero/qlementine-icons.git"
    GIT_TAG        v1.16.1
)
FetchContent_MakeAvailable(qlementine-icons)

# 2. Estándar C++23 y módulos solo para los objetivos propios del proyecto
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(UNIX AND NOT APPLE)
    set(CMAKE_CXX_MODULE_STD ON)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(-Wno-reserved-module-identifier)
    endif()
elseif(MSVC)
    add_compile_options(/utf-8)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Qml QmlPrivate QuickControls2 QuickDialogs2 Svg Network)

if(WIN32)
    find_package(cmark-gfm CONFIG REQUIRED)
    find_package(cmark-gfm-extensions CONFIG REQUIRED)
endif()

qt_standard_project_setup(REQUIRES 6.5)
qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

add_subdirectory(src/platform)
add_subdirectory(src/core)
add_subdirectory(src/libs/MarkdownViewer)
add_subdirectory(src/ui)
add_subdirectory(src/app)

if(WIN32)
    set_target_properties(FronterMind PROPERTIES WIN32_EXECUTABLE $<CONFIG:Release>)
endif()

# --- REGLAS DE INSTALACIÓN Y EMPAQUETADO (LINUX / WINDOWS) ---
install(TARGETS FronterMind
    RUNTIME DESTINATION usr/bin
    BUNDLE DESTINATION .
)

install(FILES "${CMAKE_SOURCE_DIR}/assets/icons/frontermind-icon_negative@256x256.png"
    DESTINATION usr/share/icons/hicolor/256x256/apps
    RENAME frontermind.png
    OPTIONAL
)

set(CPACK_PACKAGE_NAME "frontermind-harness")
set(CPACK_PACKAGE_VENDOR "Fabrizio Franco")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "FronterMind - Local AI Agent Harness en C++23 y Qt6")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_CONTACT "fabrizio@frontermind.local")
set(CPACK_PACKAGING_INSTALL_PREFIX "/")

if(UNIX AND NOT APPLE)
    set(CPACK_GENERATOR "TGZ;DEB")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6core6, libqt6gui6, libqt6qml6, libqt6quick6, libqt6network6, libqt6svg6, libcmark-gfm0.29.0.gfm.13")
endif()

include(CPack)
