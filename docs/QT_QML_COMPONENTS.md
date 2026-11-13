## 🏗️ Guía Maestra: Creación de Componentes Qt 6 / QML (C++ Backend)

**Propósito:** Este documento es una referencia exhaustiva para evitar los errores de enlazado (linking), registro de tipos y configuración de CMake que surgen al crear componentes personalizados (con lógica en C++) para la interfaz QML.
Audiencia: Desarrolladores y Agentes de IA que colaboren en FronterMind.

## 📁 1. Arquitectura de Directorios (Regla de Oro)

Los componentes personalizados nunca deben ir mezclados con el Core o la UI principal. Deben vivir en su propio espacio para garantizar la modularidad y el aislamiento.
text

```text
src/
├── app/              # El ejecutable final (solo main.cpp y enlaces)
├── core/             # Lógica de negocio pura (IA, Agentes, Modelos)
├── platform/         # Abstracción del SO (Linux, Windows, macOS)
├── ui/               # La interfaz principal (Vistas, Sidebar, etc.)
└── libs/             # 🔥 FÁBRICA DE COMPONENTES CUSTOM
    └── MiComponente/ # Cada componente es una librería estática independiente
        ├── include/  # Cabeceras C++
        ├── src/      # Implementación C++
        ├── qml/      # Archivos QML (o en la raíz del componente)
        └── CMakeLists.txt
```

## ⚙️ 2. Configuración de CMake (El Talón de Aquiles)

El error más común es asumir que CMake enlazará los plugins automáticamente. **En Qt 6, si usas librerías estáticas (STATIC), el enlazador descarta el código del plugin si no se le fuerza.**

### 2.1. CMakeLists.txt RAÍZ (Configuración Global)

Las políticas de Qt deben estar en la raíz para que todos los subdirectorios las hereden. Si las pones en ui/, los módulos QML se empaquetarán en rutas erróneas (`:/FronterMind/...` en vez de `:/qt/qml/...`).

```cmake

cmake_minimum_required(VERSION 3.25)
project(MiProyecto CXX)

find_package(Qt6 REQUIRED COMPONENTS Quick Qml Svg)

# 🔥 CRUCIAL: Políticas globales en la raíz
qt_standard_project_setup(REQUIRES 6.5)
qt_policy(SET QTP0001 NEW) # Fuerza el uso de :/qt/qml/
qt_policy(SET QTP0004 NEW) # Genera qmldir en subcarpetas

add_subdirectory(src/libs/MiComponente)
add_subdirectory(src/ui)
add_subdirectory(src/app)
```

### 2.2. `src/libs/MiComponente/CMakeLists.txt`

Aquí creamos la librería estática y el módulo QML.
cmake

```cmake
qt_add_library(MiComponente STATIC)

qt_add_qml_module(MiComponente
    URI Proyecto.Components.MiComponente
    VERSION 1.0
    SOURCES
        include/MiComponente.h
        src/MiComponente.cpp
    QML_FILES
        MiVista.qml
    # Asegura que los recursos se empaqueten en la ruta correcta
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Proyecto/Components/MiComponente"
)

target_include_directories(MiComponente PUBLIC include)

# Enlazar dependencias
target_link_libraries(MiComponente
    PUBLIC Qt6::Quick Qt6::Qml
    PRIVATE Qt6::Gui # Si usas QPainter, etc.
)
```

### 2.3. `src/ui/CMakeLists.txt`

La UI enlaza la librería, pero nunca el plugin.

```cmake
qt_add_library(FrontUI STATIC)

# ... qt_add_qml_module para la UI ...

target_link_libraries(FrontUI
    PUBLIC
        Qt6::Quick
        MiComponente  # <-- Se enlaza la librería base
    PRIVATE
        Qt6::QmlPrivate
)
```

### 2.4. `src/app/CMakeLists.txt (El Error Fatal)`

El ejecutable final DEBE enlazar tanto la librería como su plugin (`*plugin`). Si falta el plugin, obtendrás el error `undefined reference to qt_static_plugin_...`.

```cmake
qt_add_executable(MiApp main.cpp)

target_link_libraries(MiApp PRIVATE
    FrontUI
    FrontUIplugin             # <-- Plugin de la UI (Crucial)
    MiComponente
    MiComponenteplugin        # <-- Plugin del componente (Crucial)
)
```

## 🧠 3. C++ y QML: Registro y Uso

### 3.1. Clase C++ (`MiComponente.h`)

Para que QML reconozca la clase, usa `QML_ELEMENT` o `QML_NAMED_ELEMENT`. Si el nombre de la clase choca con el nombre del módulo QML (ej. `MiComponente`), renómbrala en `QML con QML_NAMED_ELEMENT(MiParser)`.

```cpp
#pragma once
#include <QObject>
#include <QtQml/qqmlregistration.h>

class MiParser : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(MiParser) // Nombre visible en QML
    
    Q_PROPERTY(QString texto READ texto WRITE setTexto NOTIFY textoChanged)
    // ...
};
```

### 3.2. Archivo QML (`MiVista.qml`)

El archivo QML debe llamarse igual que el tipo que quieres exportar (ej. `MiVista.qml`). No debe llevar el mismo nombre que el módulo para evitar el error `Cannot assign to non-existent property "anchors"` (que ocurre cuando QML confunde el módulo con el tipo).

```qml
import QtQuick

Item {
    id: root
    property string textoEntrada: ""
    
    // Usamos la clase C++ registrada
    MiParser {
        id: parser
        texto: root.textoEntrada
    }
    
    Text {
        textFormat: Text.RichText
        text: parser.textoHtml
    }
}
```

### 3.3. Importación en la UI (`OtraVista.qml`)

Importa el módulo sin la versión si estás usando Qt 6.5+. La versión a veces causa conflictos con el registro estático.

```qml
import Proyecto.Components.MiComponente

// Uso del componente:
MiVista {
    width: 300
    textoEntrada: "Hola Mundo"
}
```

## 🚨 4. Errores Comunes y Cómo Solucionarlos
Error en Consola	Causa	Solución
`undefined reference to 'qt_static_plugin_X'`	El linker descartó el plugin porque no se enlazó explícitamente en el ejecutable.	Añadir `Xplugin` al `target_link_libraries` del `src/app/CMakeLists.txt`.
`No module named "..." found`	Las políticas de CMake (`QTP0001`) no están en la raíz, o falta el plugin.	Mover `qt_standard_project_setup` a la raíz. Enlazar el plugin en `app/`.
`Type X is not a type`	El archivo QML no se procesó, falta `QML_ELEMENT` o el nombre del archivo choca con el módulo.	Revisar `SOURCES` y `QML_FILES` en `qt_add_qml_module`. Renombrar el `.qml` si es necesario.
`Q_IMPORT_QML_PLUGIN` da error en `main.cpp`	Las macros manuales chocan con el código que genera CMake.	Eliminar todas las macros `Q_IMPORT_QML_PLUGIN` de `main.cpp`. CMake lo hace solo si enlazas el plugin.

## 📦 5. Dependencias Externas (PkgConfig)

Si un componente usa una librería C (como `cmark-gfm`), usa `pkg-config` en su CMake. Ten en cuenta que algunas librerías separan sus extensiones (ej. `cmark-gfm-extensions`).

```cmake
find_package(PkgConfig REQUIRED)
pkg_check_modules(MI_LIB REQUIRED IMPORTED_TARGET libmi-libreria)

target_link_libraries(MiComponente PRIVATE
    PkgConfig::MI_LIB
    mi-libreria-extensions # <-- Enlazar extensiones si es necesario
)
```

## ✅ 6. Checklist para el Agente de IA antes de crear un componente:

  ¿He creado la carpeta en `src/libs/`?

  ¿El `CMakeLists.txt` raíz tiene `qt_policy` y `qt_standard_project_setup`?

  ¿La clase C++ tiene `QML_ELEMENT` o `QML_NAMED_ELEMENT`?

  ¿He añadido los `.h` y `.cpp` dentro de `SOURCES` en `qt_add_qml_module`?

  ¿He enlazado el `*plugin` en `src/app/CMakeLists.txt`?

  ¿He eliminado cualquier `Q_IMPORT_QML_PLUGIN` de `main.cpp`?

  ¿El archivo QML no tiene el mismo nombre que el módulo?
