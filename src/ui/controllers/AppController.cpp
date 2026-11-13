#include "AppController.h"
#include <QTimer>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>

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

    connect(m_agentEngine, &AgentEngine::toolExecutionStarted, this, [this](const QString &toolName) {
            if (!m_chatMessages.isEmpty() && m_chatMessages.last().startsWith("Agente: Pensando")) {
                m_chatMessages.last() = "Agente: Pensando (ejecutando " + toolName + ")...";
                emit chatMessagesChanged();
            }
    });

    connect(m_agentEngine, &AgentEngine::messageReceived, this, [this](const QString &response) {
        m_isGenerating = false;
        emit isGeneratingChanged();
        if (!m_chatMessages.isEmpty() && m_chatMessages.last().startsWith("Agente: Pensando")) {
            m_chatMessages.removeLast();
        }
        m_chatMessages.append("Agente: " + response);
        emit chatMessagesChanged();
    });

    connect(m_agentEngine, &AgentEngine::errorOccurred, this, [this](const QString &error) {
        m_isGenerating = false;
        emit isGeneratingChanged();
        if (!m_chatMessages.isEmpty() && m_chatMessages.last().startsWith("Agente: Pensando")) {
            m_chatMessages.removeLast();
        }
        m_chatMessages.append("Sistema: Error - " + error);
        emit chatMessagesChanged();
    });

    connect(m_agentEngine, &AgentEngine::finishedGenerating, this, [this]() {
        if (m_isGenerating) {
            m_isGenerating = false;
            emit isGeneratingChanged();
            if (!m_chatMessages.isEmpty() && m_chatMessages.last().startsWith("Agente: Pensando")) {
                m_chatMessages.removeLast();
                m_chatMessages.append("Sistema: Generación terminada sin respuesta.");
                emit chatMessagesChanged();
            }
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
    m_chatMessages.append("Sistema: Modelo " + alias + " seleccionado.");
    emit chatMessagesChanged();
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

    QJsonArray modelsArray;
    for (const QVariant &item : m_localModels) {
        QVariantMap map = item.toMap();
        QJsonObject obj;
        obj["path"] = map["path"].toString();
        obj["alias"] = map["alias"].toString();
        modelsArray.append(obj);
    }
    settings.setValue("models", QJsonDocument(modelsArray).toJson(QJsonDocument::Compact));
}

void AppController::loadSettings() {
    QSettings settings("FronterMind", "Harness");

    m_currentEngine = settings.value("engine", "llama.cpp").toString();
    m_enginePath = settings.value("enginePath", "/usr/bin/llama-server").toString();
    m_currentModel = settings.value("currentModel", "Ninguno").toString();
    m_maxContext = settings.value("maxContext", 4096).toInt();

    QString modelsJson = settings.value("models", "").toString();
    if (!modelsJson.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(modelsJson.toUtf8());
        if (doc.isArray()) {
            m_localModels.clear();
            QJsonArray arr = doc.array();
            for (const QJsonValue &val : arr) {
                QJsonObject obj = val.toObject();
                QVariantMap map;
                map["path"] = obj["path"].toString();
                map["alias"] = obj["alias"].toString();
                m_localModels.append(map);
            }
        }
    }

    emit currentEngineChanged();
    emit enginePathChanged();
    emit currentModelChanged();
    emit localModelsChanged();
    emit maxContextChanged();
}

void AppController::clearChat() {
    m_chatMessages.clear();
    m_agentEngine->clearMemory();
    m_usedTokens = 0;
    emit chatMessagesChanged();
    emit tokensChanged();
}
