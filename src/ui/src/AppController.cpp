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

    loadSettings(); // <-- Cargar la configuración guardada

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

    startTelemetry();
}

// --- LÓGICA DEL MOTOR ---
void AppController::setCurrentEngine(const QString &engine) {
    if (m_currentEngine == engine) return;
    m_currentEngine = engine;
    emit currentEngineChanged();
    saveSettings(); // <-- GUARDAR
}

void AppController::setEnginePath(const QString &path) {
    if (m_enginePath == path) return;
    m_enginePath = path;
    emit enginePathChanged();
    saveSettings(); // <-- GUARDAR
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
    saveSettings(); // <-- GUARDAR
}

void AppController::addModel(const QString &path, const QString &alias) {
    if (path.isEmpty() || alias.isEmpty()) return;
    QVariantMap modelData;
    modelData["path"] = path;
    modelData["alias"] = alias;
    m_localModels.append(modelData);
    emit localModelsChanged();
    saveSettings(); // <-- GUARDAR
}

// --- LÓGICA DE TELEMETRÍA ---
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

// --- PERSISTENCIA ---
void AppController::saveSettings() {
    QSettings settings("FronterMind", "Harness");

    settings.setValue("engine", m_currentEngine);
    settings.setValue("enginePath", m_enginePath);
    settings.setValue("currentModel", m_currentModel);

    // Convertimos la lista de modelos a JSON para guardarla
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

    // Notificamos a la UI que los datos iniciales ya están cargados
    emit currentEngineChanged();
    emit enginePathChanged();
    emit currentModelChanged();
    emit localModelsChanged();
}
