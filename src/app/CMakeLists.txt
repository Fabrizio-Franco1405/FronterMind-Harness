qt_add_executable(FronterMind main.cpp)
target_link_libraries(FronterMind PRIVATE
  FrontUI
  FrontUIplugin
  Qt6::Qml
)
