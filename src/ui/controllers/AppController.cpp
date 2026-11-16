// ============================================================================
// SECCIÓN 1: CABECERAS DEL CONTROLADOR PRINCIPAL
// ============================================================================
#include "AppController.h"
#include <QTimer>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QUrl>
#include <QDir>

// ============================================================================
// SECCIÓN 2: MÓDULO DE FORMATEO Y NORMALIZACIÓN (PRINCIPIO SRP DE SOLID)
// ============================================================================
namespace ChatUiHelper {

static QString normalizeFilePath(const QString &rawPath) {
    QString p = rawPath.trimmed();
    if (p.startsWith("file://")) {
        p = QUrl(p).toLocalFile();
    }
#ifdef _WIN32
    if (p.length() >= 3 && p[0] == '/' && p[1].isLetter() && p[2] == ':') {
        p.remove(0, 1);
    }
#endif
    return QDir::fromNativeSeparators(p);
}

static QString formatNaturalToolStatus(const QString &toolName, const QString &targetSummary, bool isError) {
    const QString cleanFile = QFileInfo(targetSummary).fileName().isEmpty()
                              ? targetSummary
                              : QFileInfo(targetSummary).fileName();

    if (isError) {
        return QStringLiteral("▸ Error al ejecutar %1 sobre %2 (reintentando...)").arg(toolName, cleanFile);
    }
    if (toolName == "list_directory") {
        const QString dirName = (targetSummary.isEmpty() || targetSummary == ".") ? "directorio del proyecto" : cleanFile;
        return QStringLiteral("▸ Leyendo %1.").arg(dirName);
    }
    if (toolName == "read_file") {
        return QStringLiteral("▸ Analizando contenido del archivo %1").arg(cleanFile);
    }
    if (toolName == "write_file") {
        return QStringLiteral("▸ Creando archivo %1").arg(cleanFile);
    }
    if (toolName == "edit_file") {
        return QStringLiteral("▸ Modificando archivo %1").arg(cleanFile);
    }
    if (toolName == "web_search") {
        return QStringLiteral("▸ Realizando búsqueda en internet: \"%1\"").arg(targetSummary);
    }
    if (toolName == "http_fetch") {
        return QStringLiteral("▸ Descargando y leyendo URL: %1").arg(targetSummary);
    }
    if (toolName == "run_shell_command") {
        return QStringLiteral("▸ Ejecutando en terminal: `%1`").arg(targetSummary);
    }
    return QStringLiteral("▸ Ejecutando herramienta %1 (%2)").arg(toolName, targetSummary);
}

static QString nextMonologueForTool(const QString &toolName, const QString &targetSummary) {
    if (toolName == "list_directory") {
        return QStringLiteral("Agente: Analizando estructura del proyecto...");
    }
    if (toolName == "read_file") {
        return QStringLiteral("Agente: Inspeccionando código de %1...").arg(QFileInfo(targetSummary).fileName());
    }
    if (toolName == "web_search" || toolName == "http_fetch") {
        return QStringLiteral("Agente: Procesando información obtenida de internet...");
    }
    if (toolName == "write_file" || toolName == "edit_file") {
        return QStringLiteral("Agente: Verificando cambios aplicados en el archivo...");
    }
    return QStringLiteral("Agente: Evaluando salida de %1...").arg(toolName);
}

} // namespace ChatUiHelper

// ============================================================================
// SECCIÓN 3: CONSTRUCTOR Y CONEXIONES REACTIVAS CON EL MOTOR C++ (SSE + TOOLS)
// ============================================================================
AppController::AppController(QObject *parent)
    : QObject(parent)
    , m_agentEngine(new AgentEngine(this))
    , m_engineManager(new EngineManager(this))
    , m_systemMonitor(SystemMonitor::create())
{
    loadSettings();
    m_agentEngine->setMaxContextTokens(m_maxContext);

    // --- 3.1: Inicio de generación ---
    connect(m_agentEngine, &AgentEngine::startedGenerating, this, [this]() {
        m_isGenerating = true;
        emit isGeneratingChanged();
        m_chatMessages.append("Agente: Analizando solicitud y planificando pasos...");
        emit chatMessagesChanged();
    });

    // --- 3.2: Trazas en tiempo real de herramientas ejecutadas ---
    connect(m_agentEngine, &AgentEngine::toolActionLogged, this,
            [this](const QString &toolName, const QString &targetSummary, const QString &resultPreview, bool isError) {
        Q_UNUSED(resultPreview);
        const QString stepLine = "Tool: " + ChatUiHelper::formatNaturalToolStatus(toolName, targetSummary, isError);
        const QString nextThought = ChatUiHelper::nextMonologueForTool(toolName, targetSummary);

        // Si había una burbuja de streaming o de pensamiento activa al final, insertar el paso antes
        if (!m_chatMessages.isEmpty() && m_chatMessages.last().startsWith("Agente: ")) {
            const int lastIdx = m_chatMessages.size() - 1;
            m_chatMessages.insert(lastIdx, stepLine);
            m_chatMessages.last() = nextThought;
        } else {
            m_chatMessages.append(stepLine);
            m_chatMessages.append(nextThought);
        }
        emit chatMessagesChanged();
    });

    // --- 3.3: Streaming SSE Token por Token en Tiempo Real (Latencia 0ms) ---
    connect(m_agentEngine, &AgentEngine::streamChunkReceived, this, [this](const QString &partialContent) {
        // Caso A: El modelo está pensando dentro de <think>...</think> (ej. DeepSeek-R1 / Qwen3)
        if (partialContent.contains("<think>") && !partialContent.contains("</think>")) {
            QString rawThought = partialContent;
            rawThought.remove("<think>");
            const QStringList lines = rawThought.trimmed().split('\n', Qt::SkipEmptyParts);
            const QString lastLine = lines.isEmpty() ? QStringLiteral("Razonando") : lines.last().trimmed();
            const QString preview = lastLine.length() > 80 ? lastLine.left(80) : lastLine;

            const QString thinkingBubble = QStringLiteral("Agente: %1...").arg(preview);
            if (!m_chatMessages.isEmpty() && m_chatMessages.last().startsWith("Agente: ") &&
                m_chatMessages.last().endsWith("...")) {
                m_chatMessages.last() = thinkingBubble;
            } else {
                m_chatMessages.append(thinkingBubble);
            }
            emit chatMessagesChanged();
            return;
        }

        // Caso B: El modelo ya está emitiendo la respuesta final en Markdown
        removeTemporaryStatusBubble();

        const QString liveBubble = QStringLiteral("Agente: ") + partialContent;
        if (!m_chatMessages.isEmpty() && m_chatMessages.last().startsWith("Agente: ") &&
            !m_chatMessages.last().endsWith("...")) {
            m_chatMessages.last() = liveBubble;
        } else {
            m_chatMessages.append(liveBubble);
        }
        emit chatMessagesChanged();
    });

    // --- 3.4: Consolidación de la respuesta final ---
    connect(m_agentEngine, &AgentEngine::messageReceived, this, [this](const QString &response) {
        m_isGenerating = false;
        emit isGeneratingChanged();
        removeTemporaryStatusBubble();

        const QString finalBubble = QStringLiteral("Agente: ") + response;
        if (!m_chatMessages.isEmpty() && m_chatMessages.last().startsWith("Agente: ") &&
            !m_chatMessages.last().endsWith("...")) {
            m_chatMessages.last() = finalBubble;
        } else {
            m_chatMessages.append(finalBubble);
        }
        emit chatMessagesChanged();
    });

    // --- 3.5: Manejo de errores ---
    connect(m_agentEngine, &AgentEngine::errorOccurred, this, [this](const QString &error) {
        m_isGenerating = false;
        emit isGeneratingChanged();
        removeTemporaryStatusBubble();
        m_chatMessages.append("Sistema: Error - " + error);
        emit chatMessagesChanged();
    });

    // --- 3.6: Fin del ciclo de inferencia ---
    connect(m_agentEngine, &AgentEngine::finishedGenerating, this, [this]() {
        if (m_isGenerating) {
            m_isGenerating = false;
            emit isGeneratingChanged();
            removeTemporaryStatusBubble();
            emit chatMessagesChanged();
        }
    });

    // --- 3.7: Actualización de tokens usados ---
    connect(m_agentEngine, &AgentEngine::tokensUpdated, this, [this](int prompt, int completion, int total) {
        Q_UNUSED(prompt);
        Q_UNUSED(completion);
        m_usedTokens = total;
        emit tokensChanged();
    });

    startTelemetry();
}

AppController::~AppController() {
    delete m_systemMonitor;
    m_systemMonitor = nullptr;
}

void AppController::removeTemporaryStatusBubble() {
    while (!m_chatMessages.isEmpty()) {
        const QString &lastMsg = m_chatMessages.last();
        if (lastMsg.startsWith("Agente: ") && lastMsg.endsWith("...")) {
            m_chatMessages.removeLast();
        } else {
            break;
        }
    }
}

// ============================================================================
// SECCIÓN 4: CONFIGURACIÓN DEL MOTOR DE INFERENCIA Y ESPACIO DE TRABAJO
// ============================================================================
void AppController::setCurrentEngine(const QString &engine) {
    if (m_currentEngine == engine) return;
    m_currentEngine = engine;

    if (engine.compare("Ollama", Qt::CaseInsensitive) == 0) {
        m_enginePath = "http://127.0.0.1:11434/v1/chat/completions";
        m_agentEngine->setServerUrl(m_enginePath);
    } else if (engine.compare("LM Studio", Qt::CaseInsensitive) == 0) {
        m_enginePath = "http://127.0.0.1:1234/v1/chat/completions";
        m_agentEngine->setServerUrl(m_enginePath);
    } else {
        if (m_enginePath.startsWith("http")) {
#ifdef _WIN32
            m_enginePath = "C:/llama.cpp/llama-server.exe";
#else
            m_enginePath = "/usr/bin/llama-server";
#endif
        }
        m_agentEngine->setServerUrl("http://127.0.0.1:8080/v1/chat/completions");
    }

    emit currentEngineChanged();
    emit enginePathChanged();
    saveSettings();
}

void AppController::setEnginePath(const QString &path) {
    const QString clean = path.trimmed().startsWith("http") ? path.trimmed() : ChatUiHelper::normalizeFilePath(path);
    if (m_enginePath == clean) return;
    m_enginePath = clean;

    if (m_enginePath.startsWith("http")) {
        m_agentEngine->setServerUrl(m_enginePath);
    } else {
        m_agentEngine->setServerUrl("http://127.0.0.1:8080/v1/chat/completions");
    }

    emit enginePathChanged();
    saveSettings();
}

void AppController::setMaxContext(int context) {
    if (m_maxContext == context) return;
    m_maxContext = context;
    m_agentEngine->setMaxContextTokens(context);
    emit maxContextChanged();
    saveSettings();
}

void AppController::setWorkspacePath(const QString &path) {
    const QString cleanPath = ChatUiHelper::normalizeFilePath(path);
    if (cleanPath.isEmpty() || cleanPath == m_agentEngine->workspaceDir()) return;

    m_agentEngine->setWorkspaceDir(cleanPath);
    emit workspacePathChanged();
    saveSettings();
}

// ============================================================================
// SECCIÓN 5: GESTIÓN DEL CHAT, ARCHIVOS ADJUNTOS Y HOT-SWAP DE MODELOS
// ============================================================================
void AppController::attachFile(const QString &path) {
    const QString clean = ChatUiHelper::normalizeFilePath(path);
    QFile file(clean);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_attachedFileName = QFileInfo(clean).fileName();
        m_attachedFileContent = QString::fromUtf8(file.readAll());
        emit attachmentChanged();
    } else {
        m_chatMessages.append("Sistema: No se pudo leer el archivo.");
        emit chatMessagesChanged();
    }
}

void AppController::clearAttachment() {
    m_attachedFileName.clear();
    m_attachedFileContent.clear();
    emit attachmentChanged();
}

void AppController::sendMessage(const QString &text) {
    if ((text.trimmed().isEmpty() && m_attachedFileName.isEmpty()) || m_isGenerating) return;

    if (m_currentModel == "Ninguno") {
        m_chatMessages.append("Sistema: Carga un modelo primero.");
        emit chatMessagesChanged();
        return;
    }

    QString finalMessage = text;

    if (!m_attachedFileContent.isEmpty()) {
        finalMessage = QStringLiteral("Contenido del archivo '%1':\n\n```\n%2\n```\n\n%3")
                           .arg(m_attachedFileName, m_attachedFileContent, text);
        m_chatMessages.append("Tú: [Archivo: " + m_attachedFileName + "] " + text);
        clearAttachment();
    } else {
        m_chatMessages.append("Tú: " + text);
    }

    emit chatMessagesChanged();
    m_agentEngine->sendMessage(m_currentModel, finalMessage);
}

void AppController::loadModel(const QString &alias, const QString &path) {
    const QString clean = ChatUiHelper::normalizeFilePath(path);
    const QString targetModel = alias + " (" + clean + ")";
    if (m_currentModel == targetModel) return;

    m_currentModel = targetModel;
    emit currentModelChanged();
    saveSettings();

    // Hot-Swap automático en VRAM si llama-server está en ejecución
    if (m_engineManager && m_engineManager->isRunning() && !m_enginePath.startsWith("http")) {
        m_engineManager->stopServer();
        QTimer::singleShot(450, this, [this, clean]() {
            m_engineManager->startServer(m_enginePath, clean, 99, m_maxContext);
        });
    }
}

void AppController::addModel(const QString &path, const QString &alias) {
    const QString clean = ChatUiHelper::normalizeFilePath(path);
    if (clean.isEmpty() || alias.isEmpty()) return;
    QVariantMap modelData;
    modelData["path"] = clean;
    modelData["alias"] = alias;
    m_localModels.append(modelData);
    emit localModelsChanged();
    saveSettings();
}

void AppController::removeModel(int index) {
    if (index < 0 || index >= m_localModels.size()) return;
    QVariantMap removed = m_localModels.at(index).toMap();
    m_localModels.removeAt(index);

    if (m_currentModel.startsWith(removed["alias"].toString() + " (")) {
        if (!m_localModels.isEmpty()) {
            QVariantMap first = m_localModels.first().toMap();
            loadModel(first["alias"].toString(), first["path"].toString());
        } else {
            m_currentModel = "Ninguno";
            emit currentModelChanged();
            if (m_engineManager && m_engineManager->isRunning()) {
                m_engineManager->stopServer();
            }
        }
    }

    emit localModelsChanged();
    saveSettings();
}

// ============================================================================
// SECCIÓN 6: SERVIDORES MCP, TELEMETRÍA Y PERSISTENCIA (QSETTINGS)
// ============================================================================
bool AppController::connectMcpServer(const QString &name, const QString &commandLine) {
    const QString cleanName = name.trimmed();
    const QString cleanCmd  = commandLine.trimmed();
    QStringList parts = QProcess::splitCommand(cleanCmd);
    if (cleanName.isEmpty() || parts.isEmpty()) return false;

    const QString binary = parts.takeFirst();
    QString err;
    const bool ok = m_agentEngine->addMcpServer(cleanName, binary, parts, err);
    if (ok) {
        bool exists = false;
        for (const QVariant &v : m_mcpServers) {
            if (v.toMap()["name"].toString() == cleanName) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            QVariantMap mcpMap;
            mcpMap["name"] = cleanName;
            mcpMap["command"] = cleanCmd;
            m_mcpServers.append(mcpMap);
            emit mcpServersChanged();
            saveSettings();
        }
        emit toolsChanged();
    } else {
        m_chatMessages.append("Sistema: Error al conectar MCP '" + cleanName + "' -> " + err);
        emit chatMessagesChanged();
    }
    return ok;
}

void AppController::removeMcpServer(int index) {
    if (index < 0 || index >= m_mcpServers.size()) return;
    m_mcpServers.removeAt(index);
    emit mcpServersChanged();
    saveSettings();
}

void AppController::startTelemetry() {
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        m_cpuUsage = m_systemMonitor->getCpuUsage();
        m_ramUsage = m_systemMonitor->getRamUsage();
        m_vramUsage = m_systemMonitor->getVramUsage();
        m_appMemory = m_systemMonitor->getAppMemory();
        emit telemetryChanged();
    });
    timer->start(1000);
}

void AppController::saveSettings() {
    QSettings settings("FronterMind", "Harness");

    settings.setValue("engine", m_currentEngine);
    settings.setValue("enginePath", m_enginePath);
    settings.setValue("currentModel", m_currentModel);
    settings.setValue("maxContext", m_maxContext);
    settings.setValue("workspacePath", m_agentEngine->workspaceDir());

    QJsonArray modelsArray;
    for (const QVariant &item : m_localModels) {
        QVariantMap map = item.toMap();
        QJsonObject obj;
        obj["path"] = ChatUiHelper::normalizeFilePath(map["path"].toString());
        obj["alias"] = map["alias"].toString();
        modelsArray.append(obj);
    }
    settings.setValue("models", QJsonDocument(modelsArray).toJson(QJsonDocument::Compact));

    QJsonArray mcpArray;
    for (const QVariant &item : m_mcpServers) {
        QVariantMap map = item.toMap();
        QJsonObject obj;
        obj["name"] = map["name"].toString();
        obj["command"] = map["command"].toString();
        mcpArray.append(obj);
    }
    settings.setValue("mcpServers", QJsonDocument(mcpArray).toJson(QJsonDocument::Compact));
}

void AppController::loadSettings() {
    QSettings settings("FronterMind", "Harness");

    m_currentEngine = settings.value("engine", "llama.cpp").toString();
    const QString rawEnginePath = settings.value("enginePath", "/usr/bin/llama-server").toString();
    m_enginePath = rawEnginePath.trimmed().startsWith("http") ? rawEnginePath.trimmed() : ChatUiHelper::normalizeFilePath(rawEnginePath);
    m_currentModel = settings.value("currentModel", "Ninguno").toString();
    m_maxContext = settings.value("maxContext", 4096).toInt();

    if (m_currentEngine.compare("Ollama", Qt::CaseInsensitive) == 0) {
        m_agentEngine->setServerUrl(m_enginePath.startsWith("http") ? m_enginePath : "http://127.0.0.1:11434/v1/chat/completions");
    } else if (m_currentEngine.compare("LM Studio", Qt::CaseInsensitive) == 0) {
        m_agentEngine->setServerUrl(m_enginePath.startsWith("http") ? m_enginePath : "http://127.0.0.1:1234/v1/chat/completions");
    } else {
        m_agentEngine->setServerUrl(m_enginePath.startsWith("http") ? m_enginePath : "http://127.0.0.1:8080/v1/chat/completions");
    }

    const QString savedWorkspace = ChatUiHelper::normalizeFilePath(settings.value("workspacePath", "").toString());
    if (!savedWorkspace.isEmpty()) {
        m_agentEngine->setWorkspaceDir(savedWorkspace);
    }

    const QString modelsJson = settings.value("models", "").toString();
    if (!modelsJson.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(modelsJson.toUtf8());
        if (doc.isArray()) {
            m_localModels.clear();
            for (const QJsonValue &val : doc.array()) {
                QJsonObject obj = val.toObject();
                QVariantMap map;
                const QString cleanModelPath = ChatUiHelper::normalizeFilePath(obj["path"].toString());
                const QString alias = obj["alias"].toString();
                map["path"] = cleanModelPath;
                map["alias"] = alias;
                m_localModels.append(map);

                if (m_currentModel.startsWith(alias + " (")) {
                    m_currentModel = alias + " (" + cleanModelPath + ")";
                }
            }
        }
    }

    const QString mcpJson = settings.value("mcpServers", "").toString();
    if (!mcpJson.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(mcpJson.toUtf8());
        if (doc.isArray()) {
            m_mcpServers.clear();
            for (const QJsonValue &val : doc.array()) {
                QJsonObject obj = val.toObject();
                const QString name = obj["name"].toString();
                const QString cmd  = obj["command"].toString();
                if (!name.isEmpty() && !cmd.isEmpty()) {
                    QVariantMap map;
                    map["name"] = name;
                    map["command"] = cmd;
                    m_mcpServers.append(map);
                    QTimer::singleShot(500, this, [this, name, cmd]() {
                        connectMcpServer(name, cmd);
                    });
                }
            }
        }
    }

    emit currentEngineChanged();
    emit enginePathChanged();
    emit currentModelChanged();
    emit localModelsChanged();
    emit maxContextChanged();
    emit mcpServersChanged();
    emit workspacePathChanged();
}

void AppController::clearChat() {
    m_agentEngine->stopGeneration();
    m_chatMessages.clear();
    m_agentEngine->clearMemory();
    m_usedTokens = 0;
    emit chatMessagesChanged();
    emit tokensChanged();
}
