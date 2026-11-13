qt_add_library(FrontCore STATIC
    agent/AgentEngine.h
    agent/AgentEngine.cpp
    llm/EngineManager.h
    llm/EngineManager.cpp
    context/ChatMessage.h
    context/ConversationMemory.h
    context/ConversationMemory.cpp
    tools/ITool.h
    tools/ToolRegistry.h
    tools/ToolRegistry.cpp
    tools/FileSystemTools.h
    tools/FileSystemTools.cpp
    tools/TerminalTool.cpp
    tools/TerminalTool.h
    tools/WebTools.cpp
    tools/WebTools.h
    mcp/McpClient.cpp
    mcp/McpClient.h
)

target_include_directories(FrontCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/agent
    ${CMAKE_CURRENT_SOURCE_DIR}/llm
    ${CMAKE_CURRENT_SOURCE_DIR}/context
    ${CMAKE_CURRENT_SOURCE_DIR}/tools
    ${CMAKE_CURRENT_SOURCE_DIR}/mcp
)

target_link_libraries(FrontCore
    PUBLIC
        Qt6::Core
        Qt6::Network
        FrontPlatform
)
