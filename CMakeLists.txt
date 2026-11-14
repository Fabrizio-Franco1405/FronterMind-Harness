cmake_minimum_required(VERSION 3.28)

if(UNIX AND NOT APPLE)
    set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "f35a9ac6-8463-4d38-8eec-5d6008153e7d")
endif()

project(FronterMind-Harness VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(UNIX AND NOT APPLE)
    set(CMAKE_CXX_MODULE_STD ON)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(-Wno-reserved-module-identifier)
    endif()
elseif(MSVC)
    add_compile_options(/utf-8)
endif()

include(FetchContent)
FetchContent_Declare(
    qlementine-icons
    GIT_REPOSITORY "https://github.com/oclero/qlementine-icons.git"
    GIT_TAG        v1.16.1
)
FetchContent_MakeAvailable(qlementine-icons)

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
