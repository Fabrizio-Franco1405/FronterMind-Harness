cmake_minimum_required(VERSION 3.28)

if(UNIX AND NOT APPLE)
    set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "f35a9ac6-8463-4d38-8eec-5d6008153e7d")
    if(NOT CMAKE_CXX_COMPILER)
        set(CMAKE_CXX_COMPILER /usr/bin/clang++)
        set(CMAKE_C_COMPILER /usr/bin/clang)
    endif()
elseif(WIN32)
    # 1. Autodetectar vcpkg (incluyendo el integrado en Visual Studio 2026/2022)
    if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        if(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
            set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
        else()
            foreach(VCPKG_CANDIDATE
                "C:/Program Files/Microsoft Visual Studio/18/Community/VC/vcpkg"
                "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/vcpkg"
                "C:/vcpkg"
                "C:/src/vcpkg"
                "C:/dev/vcpkg"
                "$ENV{USERPROFILE}/vcpkg"
            )
                if(EXISTS "${VCPKG_CANDIDATE}/scripts/buildsystems/vcpkg.cmake")
                    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_CANDIDATE}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
                    message(STATUS "[Windows] vcpkg detectado en: ${VCPKG_CANDIDATE}")
                    break()
                endif()
            endforeach()
        endif()
    endif()

    # 2. Autodetectar Qt6 en C:/Qt
    if(NOT DEFINED CMAKE_PREFIX_PATH AND EXISTS "C:/Qt")
        file(GLOB QT_VERSIONS LIST_DIRECTORIES true "C:/Qt/6.*")
        list(SORT QT_VERSIONS ORDER DESCENDING)
        foreach(QT_VER_DIR ${QT_VERSIONS})
            foreach(QT_COMPILER_DIR "msvc2022_64" "msvc2019_64" "llvm-mingw_64" "mingw_64")
                if(EXISTS "${QT_VER_DIR}/${QT_COMPILER_DIR}/lib/cmake/Qt6")
                    set(CMAKE_PREFIX_PATH "${QT_VER_DIR}/${QT_COMPILER_DIR}" CACHE PATH "" FORCE)
                    message(STATUS "[Windows] Qt6 detectado automaticamente en: ${QT_VER_DIR}/${QT_COMPILER_DIR}")
                    break()
                endif()
            endforeach()
            if(CMAKE_PREFIX_PATH)
                break()
            endif()
        endforeach()
    endif()
endif()

project(FronterMind-Harness VERSION 0.1.0 LANGUAGES CXX)

# --- QLEMENTINE ICONS ---
include(FetchContent)
FetchContent_Declare(
    qlementine-icons
    GIT_REPOSITORY "https://github.com/oclero/qlementine-icons.git"
    GIT_TAG        v1.16.1
)
FetchContent_MakeAvailable(qlementine-icons)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wno-reserved-module-identifier)
endif()

if(MSVC)
    add_compile_options(/utf-8 /MP)
endif()

if(UNIX AND NOT APPLE)
    set(CMAKE_CXX_MODULE_STD ON)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Qml QmlPrivate QuickDialogs2 Svg Network)

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

# --- INSTALACIÓN Y EMPAQUETADO (RELEASE v0.1.0) ---
install(TARGETS FronterMind
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

set(CPACK_PACKAGE_NAME "frontermind-harness")
set(CPACK_PACKAGE_VENDOR "Fabrizio Franco")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "FronterMind - Local AI Agent Harness en C++23 y Qt6")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_CONTACT "fabrizio@frontermind.local")

if(WIN32)
    set(CPACK_GENERATOR "ZIP")
elseif(UNIX AND NOT APPLE)
    set(CPACK_GENERATOR "DEB;TGZ")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
endif()

include(CPack)
