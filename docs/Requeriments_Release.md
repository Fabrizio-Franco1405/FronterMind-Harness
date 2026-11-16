## Lista Completa de Novedades para la Versión `v0.2.0` (Roadmap Oficial)

Para que tu proyecto siga escalando con nivel de producción y tengas claro qué construir en **`v0.2.0`**, aquí tienes la hoja de ruta dividida por áreas técnicas:

### 🧠 1. Motor de Inferencia & Rendimiento (`Core / EngineManager`)

* **Streaming de Tokens en Tiempo Real (SSE):** Mostrar las palabras de la IA token por token a medida que `llama-server` las genera (`stream: true`), en lugar de esperar a que termine el bloque completo.
* **Soporte Multi-Backend (Ollama, LM Studio y OpenAI-Compatible API):** Permitir que el selector `Motor de Inferencia` conecte tanto con un binario local `llama-server` como con endpoints HTTP ya activos (`http://localhost:11434/v1` para Ollama o servidores vLLM remotos).
* **Control Avanzado de Parámetros de Inferencia:** Deslizadores en `SettingsView` para ajustar **Temperatura (`temp`)**, **Top-P**, **Capas en GPU (`-ngl`)** (actualmente fijo en `99`) y **System Prompt personalizado**.
* **Detección Automática de Hardware para `-ngl`:** Calcular cuántas capas caben en la VRAM disponible antes de lanzar `llama-server`.

### 💬 2. Experiencia de Chat & Persistencia (`UI / ChatView`)

* **Historial de Sesiones Persistente (SQLite):** Guardar conversaciones pasadas en una base de datos local SQLite para poder retomar chats anteriores desde una lista en el Sidebar, renombrarlos o exportarlos a Markdown (`.md`).
* **Botón de "Detener Generación" (`Stop Generation`) en el Chat:** Abortar la petición `QNetworkReply` en curso si el modelo se extiende demasiado o entra en bucle.
* **Edición y Regeneración de Mensajes (`Retry / Edit`):** Botón en cada burbuja para regenerar la última respuesta del agente o editar el prompt del usuario haciendo *rollback* del contexto.
* **Soporte Multimodal (Visión / `llava` / `Qwen-VL`):** Permitir adjuntar imágenes (`.png`, `.jpg`) además de archivos de código para modelos GGUF con proyector visual (`mmproj`).

### 🛠️ 3. Seguridad Agéntica & Ecosistema MCP (`AgentEngine`)

* **Modo de Confirmación Humana (*Human-in-the-Loop*) para Herramientas Críticas:** Modal o botón de aprobación en el chat (`[Permitir] / [Denegar]`) antes de que el agente ejecute comandos destructivos en `run_shell_command`, `write_file` o `edit_file`.


* **Interruptor Individual de Herramientas (Toggle ON/OFF):** Permitir activar o desactivar herramientas específicas en la vista **Herramientas & MCP** para ahorrar tokens de contexto cuando no se necesiten.


* **Visor de Diff en Tiempo Real para `edit_file` y `write_file`:** Mostrar en la burbuja de herramienta qué líneas se añadieron (en verde) y cuáles se borraron (en rojo) cuando el agente modifica tu código fuente.
* **Catálogo Rápido de Servidores MCP (*1-Click Presets*):** Botones preconfigurados para añadir servidores MCP populares (Git, SQLite, Postgres, Brave Search, Filesystem) sin tener que escribir el comando a mano.

### 🎨 4. UI/UX & Distribución (`Platform / Packaging`)

* **Selector de Temas Funcionales en `Apariencia & Temas`:** Activar el cambio en vivo entre paletas predefinidas (*Catppuccin Mocha*, *Tokyo Night*, *Gruvbox Dark*, *Nord* y *Modo Claro*).
* **Actualizador Automático (Check for Updates):** Consultar la API de GitHub Releases al iniciar para avisar si existe una nueva versión disponible para descargar.
