cmake_minimum_required(VERSION 4.4.2)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "f35a9ac6-8463-4d38-8eec-5d6008153e7d")

set(CMAKE_CXX_COMPILER /usr/bin/clang++)
set(CMAKE_C_COMPILER /usr/bin/clang)

#set(CMAKE_CXX_FLAGS "-stdlib=libc++ -Wno-reserved-module-identifier")

project(FronterMind-Harness CXX)

add_compile_options(-Wno-reserved-module-identifier)

set(CMAKE_CXX_MODULE_STD ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt6 REQUIRED COMPONENTS Quick Qml QmlPrivate QuickDialogs2)

add_subdirectory(src/platform)
add_subdirectory(src/core)
add_subdirectory(src/ui)
add_subdirectory(src/app)
