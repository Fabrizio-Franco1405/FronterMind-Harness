#include "AppController.h"
#include <QTimer>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

AppController::AppController(QObject *parent) : QObject(parent) {
    m_agentEngine = new AgentEngine(this);
    m_engineManager = new EngineManager(this);
    m_systemMonitor = SystemMonitor::create();

    loadSettings();

    // --- CONEXIONES CON EL CORE ---
    connect(m_agentEngine, &AgentEngine::startedGenerating, this, [this]() {
        m_isGenerating = true;
        emit isGeneratingChanged();
        m_chatMessages.append("Agente: Pensando...");
        emit chatMessagesChanged();
    });

    connect(m_agentEngine, &AgentEngine::messageReceived, this, [this](const QString &response) {
        m_isGenerating = false;
        emit isGeneratingChanged();
        m_chatMessages.removeLast();
        m_chatMessages.append("Agente: " + response);
        emit chatMessagesChanged();
    });

    connect(m_agentEngine, &AgentEngine::errorOccurred, this, [this](const QString &error) {
        m_isGenerating = false;
        emit isGeneratingChanged();
        m_chatMessages.removeLast();
        m_chatMessages.append("Sistema: Error - " + error);
        emit chatMessagesChanged();
    });

    // NUEVO: Asegurar que la UI se desbloquee siempre al terminar
    connect(m_agentEngine, &AgentEngine::finishedGenerating, this, [this]() {
        if (m_isGenerating) {
            m_isGenerating = false;
            emit isGeneratingChanged();
            // Si por alguna razón se quedó en "Pensando...", lo quitamos
            if (!m_chatMessages.isEmpty() && m_chatMessages.last() == "Agente: Pensando...") {
                m_chatMessages.removeLast();
                m_chatMessages.append("Sistema: Generación terminada sin respuesta.");
                emit chatMessagesChanged();
            }
        }
    });

    // NUEVO: Conectar señal de tokens del Core
    connect(m_agentEngine, &AgentEngine::tokensUpdated, this, [this](int prompt, int completion, int total) {
        m_usedTokens = total;
        emit tokensChanged();
    });

    startTelemetry();
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
    emit maxContextChanged();
    saveSettings();
}

void AppController::sendMessage(const QString &text) {
    if (text.trimmed().isEmpty() || m_isGenerating) return;

    m_chatMessages.append("Tú: " + text);
    emit chatMessagesChanged();

    if (m_currentModel == "Ninguno") {
        m_chatMessages.append("Sistema: Carga un modelo primero.");
        emit chatMessagesChanged();
        return;
    }

    m_agentEngine->sendMessage(m_currentModel, text);
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
    settings.setValue("maxContext", m_maxContext); // Guardar contexto

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
    m_maxContext = settings.value("maxContext", 4096).toInt(); // Cargar contexto

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
