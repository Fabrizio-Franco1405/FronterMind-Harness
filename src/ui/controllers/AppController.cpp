#include "AppController.h"
#include <QTimer>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>
#include <QProcess>

AppController::AppController(QObject *parent)
    : QObject(parent)
    , m_agentEngine(new AgentEngine(this))
    , m_engineManager(new EngineManager(this))
    , m_systemMonitor(SystemMonitor::create())
{
    loadSettings();
    m_agentEngine->setMaxContextTokens(m_maxContext);

    // --- CONEXIONES CON EL CORE ---
    connect(m_agentEngine, &AgentEngine::startedGenerating, this, [this]() {
        m_isGenerating = true;
        emit isGeneratingChanged();
        m_chatMessages.append("Agente: Pensando...");
        emit chatMessagesChanged();
    });

    connect(m_agentEngine, &AgentEngine::toolActionLogged, this,
            [this](const QString &toolName, const QString &targetSummary, const QString &resultPreview, bool isError) {
        QString logEntry = QStringLiteral("Tool: %1 | %2 | %3")
                               .arg(toolName, targetSummary, isError ? ("Error: " + resultPreview) : resultPreview);

        if (!m_chatMessages.isEmpty() &&
            (m_chatMessages.last().startsWith("Agente: Pensando") || m_chatMessages.last().startsWith("Agente: Analizando"))) {
            int lastIdx = m_chatMessages.size() - 1;
            m_chatMessages.insert(lastIdx, logEntry);
            m_chatMessages.last() = "Agente: Analizando resultado de " + toolName + "...";
        } else {
            m_chatMessages.append(logEntry);
        }
        emit chatMessagesChanged();
    });

    connect(m_agentEngine, &AgentEngine::messageReceived, this, [this](const QString &response) {
        m_isGenerating = false;
        emit isGeneratingChanged();
        removeTemporaryStatusBubble();
        m_chatMessages.append("Agente: " + response);
        emit chatMessagesChanged();
    });

    connect(m_agentEngine, &AgentEngine::errorOccurred, this, [this](const QString &error) {
        m_isGenerating = false;
        emit isGeneratingChanged();
        removeTemporaryStatusBubble();
        m_chatMessages.append("Sistema: Error - " + error);
        emit chatMessagesChanged();
    });

    connect(m_agentEngine, &AgentEngine::finishedGenerating, this, [this]() {
        if (m_isGenerating) {
            m_isGenerating = false;
            emit isGeneratingChanged();
            removeTemporaryStatusBubble();
            emit chatMessagesChanged();
        }
    });

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
    if (!m_chatMessages.isEmpty()) {
        const QString &lastMsg = m_chatMessages.last();
        if (lastMsg.startsWith("Agente: Pensando") || lastMsg.startsWith("Agente: Analizando")) {
            m_chatMessages.removeLast();
        }
    }
}

void AppController::setCurrentEngine(const QString &engine) {
    if (m_currentEngine == engine) return;
    m_currentEngine = engine;
    emit currentEngineChanged();
    saveSettings();
}

void AppController::setEnginePath(const QString &path) {
    if (m_enginePath == path) return;
    m_enginePath = path;
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
    QString cleanPath = path;
    cleanPath.replace("file://", "");
    if (cleanPath.isEmpty() || cleanPath == m_agentEngine->workspaceDir()) return;

    m_agentEngine->setWorkspaceDir(cleanPath);
    emit workspacePathChanged();
    saveSettings();
}

void AppController::attachFile(const QString &path) {
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_attachedFileName = QFileInfo(path).fileName();
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
    m_currentModel = alias + " (" + path + ")";
    emit currentModelChanged();
    saveSettings();
}

void AppController::addModel(const QString &path, const QString &alias) {
    if (path.isEmpty() || alias.isEmpty()) return;
    QVariantMap modelData;
    modelData["path"] = path;
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
            m_currentModel = first["alias"].toString() + " (" + first["path"].toString() + ")";
        } else {
            m_currentModel = "Ninguno";
        }
        emit currentModelChanged();
    }

    emit localModelsChanged();
    saveSettings();
}

bool AppController::connectMcpServer(const QString &name, const QString &commandLine) {
    const QString cleanName = name.trimmed();
    const QString cleanCmd  = commandLine.trimmed();
    QStringList parts = QProcess::splitCommand(cleanCmd);
    if (cleanName.isEmpty() || parts.isEmpty()) return false;

    const QString binary = parts.takeFirst();
    QString err;
    const bool ok = m_agentEngine->addMcpServer(cleanName, binary, parts, err);
    if (ok) {
        // Evitar duplicados si se reconecta desde QSettings
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
        obj["path"] = map["path"].toString();
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
    m_enginePath = settings.value("enginePath", "/usr/bin/llama-server").toString();
    m_currentModel = settings.value("currentModel", "Ninguno").toString();
    m_maxContext = settings.value("maxContext", 4096).toInt();

    const QString savedWorkspace = settings.value("workspacePath", "").toString();
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
                map["path"] = obj["path"].toString();
                map["alias"] = obj["alias"].toString();
                m_localModels.append(map);
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
                    // Reconectar automáticamente en segundo plano al iniciar
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
    m_chatMessages.clear();
    m_agentEngine->clearMemory();
    m_usedTokens = 0;
    emit chatMessagesChanged();
    emit tokensChanged();
}
