find_package(Qt6 REQUIRED COMPONENTS Core Network)

add_library(FrontCore STATIC)

target_sources(FrontCore PRIVATE
    src/AgentEngine.cpp
    src/EngineManager.cpp
    include/AgentEngine.h
    include/EngineManager.h
)

target_include_directories(FrontCore PUBLIC include)
target_link_libraries(FrontCore PUBLIC FrontPlatform Qt6::Core Qt6::Network)
