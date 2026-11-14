qt_add_library(FrontPlatform STATIC
    SystemMonitor.h
)

if(WIN32)
    target_sources(FrontPlatform PRIVATE SystemMonitorWindows.cpp)
    target_link_libraries(FrontPlatform PRIVATE dxgi psapi pdh)
elseif(UNIX AND NOT APPLE)
    target_sources(FrontPlatform PRIVATE SystemMonitorLinux.cpp)
endif()

target_include_directories(FrontPlatform PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(FrontPlatform PUBLIC Qt6::Core)
