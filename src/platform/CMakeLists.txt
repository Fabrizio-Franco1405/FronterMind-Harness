find_package(Qt6 REQUIRED COMPONENTS Core)

add_library(FrontPlatform STATIC)

target_include_directories(FrontPlatform PUBLIC include)
target_link_libraries(FrontPlatform PUBLIC Qt6::Core)

# Como los archivos están directamente en src/, ajustamos las rutas
if(WIN32)
    target_sources(FrontPlatform PRIVATE src/SystemMonitorWindows.cpp)
    target_link_libraries(FrontPlatform PRIVATE psapi)
elseif(APPLE)
    # target_sources(FrontPlatform PRIVATE src/SystemMonitorMacos.cpp)
else()
    # Linux por defecto
    target_sources(FrontPlatform PRIVATE src/SystemMonitorLinux.cpp)
endif()
