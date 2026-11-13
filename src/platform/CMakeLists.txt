find_package(Qt6 REQUIRED COMPONENTS Core)

add_library(FrontPlatform STATIC
    SystemMonitor.h
)

target_include_directories(FrontPlatform PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(FrontPlatform PUBLIC Qt6::Core)

if(WIN32)
    target_sources(FrontPlatform PRIVATE SystemMonitorWindows.cpp)
    target_link_libraries(FrontPlatform PRIVATE psapi dxgi)
elseif(APPLE)
    # target_sources(FrontPlatform PRIVATE SystemMonitorMacos.cpp)
else()
    target_sources(FrontPlatform PRIVATE SystemMonitorLinux.cpp)
endif()
