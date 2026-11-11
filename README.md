<div align="center">
  <br>
  <h1>⚙️ C++23 Modules Template</h1>
  <p><strong>Plantilla moderna y minimalista para crear proyectos en C++ utilizando Módulos C++23 con CMake, Ninja y Clang.</strong></p>
  <p>
    <img src="https://img.shields.io/badge/C%2B%2B-23-00599C?style=flat&logo=c%2B%2B" alt="C++23">
    <img src="https://img.shields.io/badge/CMake-4.4+-064F8C?style=flat&logo=cmake" alt="CMake 4.4+">
    <img src="https://img.shields.io/badge/Clang-19+-5C4EE5?style=flat&logo=llvm" alt="Clang 19+">
    <img src="https://img.shields.io/badge/Ninja-1.12+-63E6BE?style=flat" alt="Ninja 1.12+">
    <img src="https://img.shields.io/badge/license-MIT-blue?style=flat" alt="License MIT">
  </p>
  <br>
</div>

---

## Indice

- [Descripcion](#descripcion)
- [Requisitos del sistema](#requisitos-del-sistema)
- [Estructura del proyecto](#estructura-del-proyecto)
- [Guia de instalacion](#guia-de-instalacion)
  - [1. Instalar Clang con soporte de módulos](#1-instalar-clang-con-soporte-de-m%C3%B3dulos)
  - [2. Instalar CMake](#2-instalar-cmake)
  - [3. Instalar Ninja](#3-instalar-ninja)
  - [4. Clonar el repositorio](#4-clonar-el-repositorio)
  - [5. Generar el proyecto](#5-generar-el-proyecto)
  - [6. Compilar y ejecutar](#6-compilar-y-ejecutar)
  - [Compilación automática en tu editor](#compilaci%C3%B3n-autom%C3%A1tica-en-tu-editor)
- [Personalizacion](#personalizacion)
- [Solucion de problemas](#solucion-de-problemas)
- [Licencia](#licencia)

---

## Descripcion

Esta plantilla te permite **empezar a programar con módulos de C++23 en cuestion de segundos** sin perder tiempo configurando flags de compilación ni experimentando con variables de CMake. Incluye:

- **`import std;`** — importa la biblioteca estándar completa como un solo módulo.
- **CMake 4.4+** con la variable experimental `CMAKE_EXPERIMENTAL_CXX_IMPORT_STD` habilitada.
- **Clang 19+** con `-stdlib=libc++` para soporte completo de módulos C++23.
- **Ninja** como generador del _build system_ (rápido y eficiente).
- **`compile_commands.json`** exportado automáticamente (ideal para _LSPs_ como clangd, zed, vscode, etc).
- **Compilación con C++23** — usa todas las características modernas del estándar.
- **Licencia MIT** (ver `LICENSE`).

Todo está integrado y funcionando: compilas y ejecutas `Hola desde C++23 con módulos!`.

---

## Requisitos del sistema

| Herramienta               | Version minima  | Descripcion                                              |
| ------------------------- | --------------- | -------------------------------------------------------- |
| **Linux / macOS**         | —               | Sistema operativo soportado                              |
| **Clang**                 | 19+             | Compilador con soporte de módulos C++23                  |
| **libc++**                | 19+             | Biblioteca estándar compatible con módulos               |
| **CMake**                 | 4.4.2           | Generador del _build system_                             |
| **Ninja**                 | 1.12+           | Generador de compilación rápido                          |
| **Git**                   | 2.30+           | Control de versiones y clonado del repositorio           |

> **Nota:** En macOS puedes instalar Clang via `xcode-select --install` o con Homebrew (`brew install llvm`). En Windows, usa WSL2 o instala LLVM directamente desde [llvm.org](https://llvm.org/). No olvides sustituir el ""f35a9ac6-8463-4d38-8eec-5d6008153e7d" por el correspondiente en tu versión de CMake.

---

## Estructura del proyecto

```
CPP23_Modules_Template/
├── CMakeLists.txt          # Configuracion del build con CMake
├── main.cpp                # Punto de entrada con import std;
├── build/                  # Directorio de salida de la compilacion (ignorado)
│   ├── CPP23_Modules_Template            # Ejecutable compilado
│   ├── compile_commands.json
│   └── ...
└── README.md               # Este archivo
```

### Descripcion de archivos clave

| Archivo           | Proposito                                                              |
| ----------------- | ---------------------------------------------------------------------- |
| `CMakeLists.txt`  | Declara el proyecto, configura Clang, activa módulos C++23 y enlaza.   |
| `main.cpp`        | Contiene el código fuente con `import std;` y la función `main()`.    |
| `build/`          | Contiene los artefactos de compilación (ejecutable, objetos, etc).    |

---

## Guia de instalacion

### 1. Instalar Clang con soporte de módulos

#### Linux (Arch / Manjaro)

```bash
sudo pacman -S clang libc++
```

#### Linux (Ubuntu / Debian)

```bash
sudo apt update
sudo apt install clang libc++-dev libc++abi-dev lld
```

#### macOS (Homebrew)

```bash
brew install llvm
```

> En macOS, después de instalar con Homebrew, necesitás agregar las rutas al `PATH`:
> ```bash
> export PATH="/opt/homebrew/opt/llvm/bin:$PATH"
> export LDFLAGS="-L/opt/homebrew/opt/llvm/lib"
> export CPPFLAGS="-I/opt/homebrew/opt/llvm/include"
> ```

#### Windows (WSL2)

```bash
sudo apt update && sudo apt install clang libc++-dev libc++abi-dev lld
```

#### Verificar instalación

```bash
clang++ --version
```

Debe mostrar versión **19.0** o superior.

---

### 2. Instalar CMake

#### Linux

```bash
sudo apt install cmake
```

#### macOS

```bash
brew install cmake
```

#### Verificar

```bash
cmake --version
```

Se requiere **4.4.2** o superior.

---

### 3. Instalar Ninja

#### Linux

```bash
sudo apt install ninja-build
```

#### macOS

```bash
brew install ninja
```

#### Verificar

```bash
ninja --version
```

---

### 4. Clonar el repositorio

```bash
git clone https://github.com/tu-usuario/CPP23_Modules_Template.git
cd CPP23_Modules_Template
```

> Reemplaza la URL por la de tu repositorio real.

---

### 5. Generar el proyecto

Desde la raíz del repositorio, ejecuta:

```bash
cmake -G Ninja -B build -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang
```

Este comando hará lo siguiente:

1. **Configura Clang** como compilador C y C++.
2. **Genera los archivos Ninja** dentro del directorio `build/`.
3. **Habilita módulos C++23** a través de las variables en `CMakeLists.txt`.
4. **Exporta `compile_commands.json`** para integración con LSPs.

Si todo sale bien verás un mensaje como:

```
-- The CXX compiler identification is Clang 19.x.x
-- Configuring done (X.Xs)
-- Generating done (X.Xs)
-- Build files have been written to: /path/to/build
```

---

### 6. Compilar y ejecutar

Compila el proyecto con:

```bash
cmake --build build
```

El ejecutable se generará en:

```
build/CPP23_Modules_Template
```

Para ejecutarlo:

```bash
./build/CPP23_Modules_Template
```

Salida esperada:

```
Hola desde C++23 con módulos!
```

---

### Resumen visual del flujo de trabajo

```
[git clone]
     │
     v
[cmake -G Ninja -B build ...]  ← Configura Clang + módulos C++23
     │
     v
[cmake --build build        ]  ← Ninja compila main.cpp con import std;
     │
     v
[./build/CPP23_Modules_Template ]  ← ¡Hola desde C++23 con módulos!
```

---

### Compilación automática en tu editor

Si querés que tu editor compile automáticamente con CMake al guardar, dependiendo de cuál uses:

#### Zed (via `tasks.json`)

Zed no trae una extensión de CMake por defecto, pero podés configurar una **tarea** que ejecute la configuración y compilación. Crea el archivo `.zed/tasks.json` en la raíz del proyecto:

```json
{
  "label": "Build (CMake + Ninja)",
  "command": "sh",
  "args": [
    "-c",
    "cmake -G Ninja -B build -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang && cmake --build build"
  ],
  "reveal": "always",
  "hide": "never"
}
```

Para que se ejecute en cada guardado, agregá el evento en `.zed/tasks.json`:

```json
{
  "tasks": [
    {
      "label": "Build (CMake + Ninja)",
      "command": "sh",
      "args": [
        "-c",
        "cmake -G Ninja -B build -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang && cmake --build build"
      ],
      "env": {},
      "reveal": "always",
      "autosave": "on_change"
    }
  ]
}
```

> **Nota:** `autosave: "on_change"` ejecuta la tarea automáticamente cada vez que guardás un archivo, dándote compilación en tiempo real. También podés dispararla manualmente con `Ctrl+Shift+P` → "Tasks: Spawn".

#### VS Code (extension de CMake)

Para VS Code no hace falta nada de esto: instalá la extensión **CMake Tools** (`ms-vscode.cmake-tools`) y activá el auto-build. La extensión se encarga de detectar el `CMakeLists.txt`, configurar el compilador y compilar automáticamente al guardar.

Configurá el `settings.json` para habilitar la compilación con Clang + Ninja:

```json
{
  "cmake.buildDirectory": "${workspaceFolder}/build",
  "cmake.generator": "Ninja",
  "cmake.configureSettings": {
    "CMAKE_CXX_COMPILER": "clang++",
    "CMAKE_C_COMPILER": "clang"
  }
}
```

---

## Personalizacion

### Agregar un nuevo módulo

Crea un archivo `.cppm` (extensión recomendada para módulos de interfaz) en una carpeta `modules/`:

```
CPP23_Modules_Template/
├── CMakeLists.txt
├── main.cpp
├── modules/
│   └── mi_modulo.cppm
└── build/
```

Contenido de `modules/mi_modulo.cppm`:

```cpp
export module mi_modulo;

export int saludo() {
    return 42;
}
```

En `CMakeLists.txt`, registra el módulo:

```cmake
add_library(mi_modulo modules/mi_modulo.cppm)
target_sources(mi_modulo PUBLIC FILE_SET CXX_MODULES FILES modules/mi_modulo.cppm)
target_link_libraries(CPP23_Modules_Template PRIVATE mi_modulo)
```

En `main.cpp`:

```cpp
import std;
import mi_modulo;

int main() {
    std::println("El valor es: {}", saludo());
}
```

---

### Cambiar el nombre del proyecto

En `CMakeLists.txt`, modifica la línea:

```cmake
project(CPP23_Modules_Template CXX)
```

por el nombre que desees:

```cmake
project(MiProyecto CXX)
```

Recuerda también renombrar el ejecutable en `add_executable`:

```cmake
add_executable(MiProyecto main.cpp)
```

---

### Agregar dependencias externas

Puedes usar `FetchContent` para descargar dependencias automáticamente:

```cmake
include(FetchContent)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 10.2.1
)
FetchContent_MakeAvailable(fmt)

target_link_libraries(CPP23_Modules_Template PRIVATE fmt::fmt)
```

---

### Habilitar más warnings

Agrega a `CMakeLists.txt` después de `set(CMAKE_CXX_FLAGS ...)`:

```cmake
set(CMAKE_CXX_FLAGS "-stdlib=libc++ -Wno-reserved-module-identifier -Wall -Wextra -Wpedantic")
```

---

## Solucion de problemas

### Error: `module file not found` o `could not find module`

Asegúrate de que:
- Estás usando **Clang 19+** (no GCC ni MSVC).
- `CMAKE_EXPERIMENTAL_CXX_IMPORT_STD` está configurado correctamente.
- Ejecutaste `cmake --build build` después de cualquier cambio en `CMakeLists.txt`.

### Error: `stdlib=libc++ not found`

Instalá libc++:
```bash
# Ubuntu / Debian
sudo apt install libc++-dev libc++abi-dev

# macOS
brew install llvm  # incluye libc++
```

### `compile_commands.json` no se genera

Verificá que esta línea esté en `CMakeLists.txt`:
```cmake
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
```

### El ejecutable no encuentra la librería estándar

En Linux, puede ser necesario agregar la ruta de libc++ al `LD_LIBRARY_PATH`:
```bash
export LD_LIBRARY_PATH=/usr/lib/llvm-19/lib:$LD_LIBRARY_PATH
```

---

## Licencia

Este proyecto se distribuye bajo la licencia **MIT**. Consulta el archivo `LICENSE` para más detalles.

---

<div align="center">
  <sub>Hecho con ❤️ usando <a href="https://clang.llvm.org/">Clang</a>, <a href="https://cmake.org/">CMake</a> y <a href="https://ninja-build.org/">Ninja</a>.</sub>
</div>
