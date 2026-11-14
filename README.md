<div align="center">

<img src="assets/icons/frontermind-icon_negative@256x256.svg" alt="FronterMind Logo" width="110" height="110" />

# FronterMind Harness

**El Entorno de Ejecución (Harness) Nativo, Ultrarrápido y Modular para Agentes de IA Locales**

[![C++23](https://img.shields.io/badge/Standard-C%2B%2B23-00599C?style=for-the-badge&logo=c%2B%2B)](https://en.cppreference.com/w/cpp/23)
[![Qt 6](https://img.shields.io/badge/GUI-Qt6%20%2F%20QML-41CD52?style=for-the-badge&logo=qt)](https://www.qt.io/)
[![MCP Ready](https://img.shields.io/badge/Protocol-MCP%20JSON--RPC%202.0-cba6f7?style=for-the-badge)](https://modelcontextprotocol.io/)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows-89b4fa?style=for-the-badge)](#compilaci%C3%B3n-e-instalaci%C3%B3n)
[![License](https://img.shields.io/badge/License-MIT-a6e3a1?style=for-the-badge)](LICENSE)

*Cero Electron. Cero telemetría en la nube. Latencia cero en C++23 nativo.*

[Características](#-caracter%C3%ADsticas-principales) •
[Arquitectura](#-arquitectura-del-sistema) •
[Herramientas & MCP](#-herramientas-nativas-y-ecosistema-mcp) •
[Compilación](#-compilaci%C3%B3n-e-instalaci%C3%B3n) •
[Documentación](docs/)

</div>

---

## ¿Por qué FronterMind Harness?

La mayoría de las interfaces actuales para modelos de lenguaje locales son simples clientes de chat construidos sobre Electron o wrappers web pesados que consumen más de **1.5 GB de RAM** antes siquiera de cargar un modelo. Además, carecen de un motor agéntico real que permita al modelo interactuar con el sistema operativo y tus proyectos de forma transparente.

**FronterMind Harness** nace con una filosofía de ingeniería radicalmente distinta:
1. **Rendimiento Bare-Metal:** Escrito desde cero en **C++23** con renderizado acelerado por hardware en **Qt6 / QML**. Arranca en milisegundos y mantiene una huella de memoria de apenas **~180 MB de RAM**, liberando el 100% de tu VRAM y memoria del sistema para la inferencia del LLM.

2. **Verdadera Capacidad Agéntica (ReAct):** No solo conversa; **actúa**. Transforma cualquier modelo local compatible con *Tool Calling* (como **Qwen 3**, **Llama 3.1/3.3**, **DeepSeek**, **Mistral**) en un ingeniero de software autónomo capaz de explorar repositorios, leer y editar código fuente, ejecutar compilaciones en terminal y buscar documentación actualizada en internet.

3. **Transparencia Total de Acciones (*Action Traceability*):** A diferencia de las cajas negras, cada herramienta invocada por el agente genera una traza visual en tiempo real dentro del flujo de trabajo, mostrando exactamente qué directorio inspecciona, qué archivo modifica o qué comando ejecuta.

---

## Características Principales

### Orquestador Agéntico ReAct con Memoria Contextual
* **Ciclo Autónomo Multi-Paso:** El motor (`AgentEngine`) evalúa las solicitudes del modelo, ejecuta herramientas locales o remotas, inyecta los resultados de vuelta en el contexto y continúa razonando hasta resolver la tarea.

* **Ventana Deslizante Inteligente (*Sliding Window*):** Control estricto del presupuesto de contexto (`4K`, `8K`, `16K`, `32K+` tokens) con poda automática de turnos antiguos preservando siempre el *System Prompt* y la coherencia de las llamadas a herramientas.

* **Optimización para Modelos Razonadores (`<think>` stripping):** Filtra automáticamente los bloques de pensamiento interno del historial de memoria tras cada turno, reduciendo el consumo de ventana de contexto hasta en un **70%** en sesiones extensas.

### Gestión de Procesos e Inferencia Local
* **Controlador de Ciclo de Vida (`EngineManager`):** Inicia, supervisa y detiene instancias locales de `llama-server` (`llama.cpp`) con soporte automático para plantillas `--jinja`, aceleración por GPU (`-ngl`) y parada limpia en dos fases (`SIGTERM` con *graceful timeout* hacia `SIGKILL` y protección contra procesos huérfanos en Linux mediante `PR_SET_PDEATHSIG`).

* **Consola de Telemetría en Tiempo Real:** Visualización en vivo de `stdout` y `stderr` del motor de inferencia con copiado rápido para diagnóstico inmediato.

### Telemetría de Hardware de Bajo Nivel
* **Monitoreo Nativo sin Dependencias Externas:** Lectura directa del kernel para mostrar el uso de **CPU**, **RAM del sistema**, **Memoria del proceso** y **VRAM**.
* **Autodetección Inteligente de GPU Dedicada (dGPU vs iGPU):** Prioriza automáticamente tarjetas gráficas dedicadas sobre gráficos integrados tanto en **Linux** (vía `/sys/class/drm` y `mem_info_vram_total`) como en **Windows** (vía API nativa `DXGI 1.4` / `IDXGIAdapter3`).

### Motor de Renderizado Markdown & UI Personalizable
* **Parser AST Nativo (`libcmark-gfm`):** Renderizado instantáneo de tablas, listas y bloques de código con resaltado de sintaxis en C++ y copiado en un clic.
* **Motor de Temas Reactivo (`ThemeManager`):** Incluye 5 paletas diseñadas para largas jornadas de programación (*Catppuccin Mocha*, *Tokyo Night*, *Rose Pine*, *Gruvbox Dark* y *Nord Polar*), editor hexadecimal por propiedad y selector de fuentes tipográficas de interfaz y código en tiempo real.

---

## Herramientas Nativas y Ecosistema MCP

FronterMind incorpora **7 herramientas nativas compiladas en C++** listas para funcionar sin instalar runtimes externos, además de un **Cliente MCP (Model Context Protocol)** completo:

| Herramienta | Tipo | Descripción |
| :--- | :---: | :--- |
| `list_directory` | `Nativa C++` | Inspecciona cualquier directorio del *Workspace* activo detallando subcarpetas y tamaños de archivos. |
| `read_file` | `Nativa C++` | Lee código fuente o archivos de texto con numeración de líneas y soporte de rangos (`start_line` / `end_line`). |
| `write_file` | `Nativa C++` | Crea archivos nuevos o sobrescribe existentes generando automáticamente la estructura de directorios padre. |
| `edit_file` | `Nativa C++` | Realiza reemplazos quirúrgicos de bloques de código (`search_text` -> `replace_text`) sin reescribir todo el archivo. |
| `run_shell_command` | `Nativa C++` | Ejecuta comandos del sistema (`cmake`, `git`, `ninja`, tests) con captura de `stdout`/`stderr`, código de salida y *timeout* de seguridad de 30s. |
| `web_search` | `Nativa C++` | Consulta motores de búsqueda en tiempo real devolviendo títulos, URLs limpias y resúmenes actualizados sin requerir API Keys de pago. |
| `http_fetch` | `Nativa C++` | Descarga páginas web o endpoints JSON y limpia el DOM HTML para entregar texto puro optimizado en tokens. |
| **Puente MCP** | `JSON-RPC 2.0` | Conecta cualquier servidor **Model Context Protocol** externo vía `stdio` (`npx`, `uvx`, binarios locales), descubre sus herramientas con `tools/list` y las expone automáticamente al LLM. |

---

## Arquitectura del Sistema

El proyecto está diseñado bajo una separación estricta de responsabilidades donde el **Core (`src/core`) es 100% independiente de la interfaz gráfica (`src/ui`)**, permitiendo máxima escalabilidad y mantenibilidad:

```text
FronterMind-Harness/
├── assets/
│   ├── icons/                # Iconos oficiales multiplataforma (.svg, .png, .ico, .rc)
│   └── resources.qrc         # Recursos gráficos compilados en el binario
├── docs/                     # Manuales técnicos de arquitectura, compilación y releases
└── src/
    ├── platform/             # Abstracción de hardware por SO (Linux /proc & DRM | Windows Win32 & DXGI)
    ├── core/                 # Backend puro (C++23, QtCore, QtNetwork) — Cero dependencias de UI
    │   ├── agent/            # AgentEngine: Orquestador ReAct y cliente HTTP OpenAI-compatible
    │   ├── context/          # ConversationMemory & ChatMessage: Ventana deslizante y estimador BPE
    │   ├── llm/              # EngineManager: Supervisión de subprocesos llama-server
    │   ├── tools/            # ITool, ToolRegistry, FileSystemTools, TerminalTool y WebTools
    │   └── mcp/              # McpClient & McpBridgeTool: Cliente JSON-RPC 2.0 sobre stdio
    ├── libs/
    │   └── MarkdownViewer/   # Módulo estático C++/QML con libcmark-gfm y Syntax Highlighter
    ├── ui/                   # Frontend QtQuick / QML y Controladores puente (AppController, ThemeManager)
    │   ├── controllers/      # Puentes Q_PROPERTY / Q_INVOKABLE e IconImageProvider (Qlementine)
    │   └── qml/              # Vistas modulares (ChatView, SettingsView, ToolsMcpView, AppearanceView)
    └── app/                  # Punto de entrada de la aplicación (main.cpp)
```

## Compilación e Instalación

### Requisitos Previos

  - **Compilador C++23:** `Clang 17+` o `GCC 13+` en Linux | `MSVC 2022` o `Clang-cl` en Windows.

  - **Sistema de Construcción:** `CMake 3.28+` y `Ninja`.

  - **Framework:** `Qt 6.5+` (Módulos: `Quick`, `Qml`, `QuickDialogs2`, `Svg`, `Network`).

  - **Librería Markdown:** `cmark-gfm` (`libcmark-gfm` en Linux o vía `vcpkg` en Windows).

1. Compilación en Linux (Arch / CachyOS / Ubuntu / Fedora)

```Bash
# 1. Clonar el repositorio
git clone [https://github.com/tu-usuario/FronterMind-Harness.git](https://github.com/tu-usuario/FronterMind-Harness.git)
cd FronterMind-Harness

# 2. Configurar y compilar en modo Release (Optimizado)
cmake --preset linux-release
cmake --build --preset linux-release

# 3. Ejecutar
./build-release/src/app/FronterMind
```

### Generar paquetes instalables `.deb` y `.tar.gz` en Linux:

```Bash
cd build-release && cpack
```

2. Compilación en Windows (10 / 11)

En Windows, el proyecto utiliza tu instalación local de **Qt6** y emplea **vcpkg** exclusivamente para enlazar `cmark-gfm` en segundos (sin recompilar Qt):

```PowerShell
# 1. Abrir "x64 Native Tools Command Prompt for VS 2022" o PowerShell
$env:CMAKE_PREFIX_PATH="C:\Qt\6.8.0\msvc2022_64"

# 2. Configurar y compilar con el preset de Windows
cmake --preset windows-release -DVCPKG_TARGET_TRIPLET=x64-windows
cmake --build --preset windows-release

# 3. Ejecutar
.\build-win-release\src\app\FronterMind.exe
```

Para instrucciones detalladas sobre cómo generar el instalador portable `.zip` con `windeployqt`, crear paquetes `.AppImage` o gestionar los assets gráficos, consulta la documentación técnica en la carpeta `docs/`.

### Hoja de Ruta (Roadmap)

  [x] Fase 1: Gestión de memoria multi-turno y presupuesto dinámico de ventana de contexto.

  [x] Fase 2: Motor de herramientas nativas de Sistema de Archivos (read_file, write_file, edit_file, list_directory).

  [x] Fase 3: Ejecución controlada de comandos de terminal (run_shell_command) con timeout.

  [x] Fase 4: Conectividad a Internet en tiempo real (web_search y http_fetch).

  [x] Fase 5: Cliente nativo MCP (Model Context Protocol) sobre stdio y motor de temas personalizable.

  [ ] Fase 6 (Próximamente): Streaming SSE token a token en tiempo real y soporte multi-sesión con persistencia en SQLite.

### Licencia

Distribuido bajo la Licencia MIT. Consulta el archivo LICENSE para más información.
