# Al usar INTERFACE, no intentará buscar un archivo .a inexistente
add_library(FrontPlatform INTERFACE)
target_include_directories(FrontPlatform INTERFACE include)
