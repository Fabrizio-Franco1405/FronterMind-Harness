#pragma once
#include <QObject>
#include <QStringList>
#include <QVariantList>
#include "AgentEngine.h"
#include "EngineManager.h"
#include "SystemMonitor.h"

class AppController : public QObject {
    Q_OBJECT
    // Telemetría
    Q_PROPERTY(QString cpuUsage READ cpuUsage NOTIFY telemetryChanged)
    Q_PROPERTY(QString ramUsage READ ramUsage NOTIFY telemetryChanged)
    Q_PROPERTY(QString vramUsage READ vramUsage NOTIFY telemetryChanged)
    Q_PROPERTY(QString appMemory READ appMemory NOTIFY telemetryChanged)

    // Motores y Modelos
    Q_PROPERTY(QString currentEngine READ currentEngine WRITE setCurrentEngine NOTIFY currentEngineChanged)
    Q_PROPERTY(QString enginePath READ enginePath WRITE setEnginePath NOTIFY enginePathChanged) // <-- DEVUELTO
    Q_PROPERTY(QStringList availableEngines READ availableEngines CONSTANT)
    Q_PROPERTY(QString currentModel READ currentModel NOTIFY currentModelChanged)
    Q_PROPERTY(QVariantList localModels READ localModels NOTIFY localModelsChanged)
    Q_PROPERTY(EngineManager* engineManager READ engineManager CONSTANT)

    // Chat
    Q_PROPERTY(QStringList chatMessages READ chatMessages NOTIFY chatMessagesChanged)
    Q_PROPERTY(bool isGenerating READ isGenerating NOTIFY isGeneratingChanged)

public:
    explicit AppController(QObject *parent = nullptr);

    QString cpuUsage() const { return m_cpuUsage; }
    QString ramUsage() const { return m_ramUsage; }
    QString vramUsage() const { return m_vramUsage; }
    QString appMemory() const { return m_appMemory; }

    QString currentEngine() const { return m_currentEngine; }
    void setCurrentEngine(const QString &engine);
    QString enginePath() const { return m_enginePath; }
    void setEnginePath(const QString &path); // <-- DEVUELTO
    QStringList availableEngines() const { return m_availableEngines; }

    QString currentModel() const { return m_currentModel; }
    QVariantList localModels() const { return m_localModels; }
    QStringList chatMessages() const { return m_chatMessages; }
    bool isGenerating() const { return m_isGenerating; }
    EngineManager* engineManager() { return m_engineManager; }

    Q_INVOKABLE void sendMessage(const QString &text);
    Q_INVOKABLE void addModel(const QString &path, const QString &alias);
    Q_INVOKABLE void loadModel(const QString &alias, const QString &path);

signals:
    void telemetryChanged();
    void currentEngineChanged();
    void enginePathChanged(); // <-- DEVUELTO
    void currentModelChanged();
    void localModelsChanged();
    void chatMessagesChanged();
    void isGeneratingChanged();

private:
    void startTelemetry();
    void loadSettings(); // <-- NUEVO
    void saveSettings(); // <-- NUEVO

    AgentEngine *m_agentEngine;
    EngineManager *m_engineManager;
    SystemMonitor *m_systemMonitor;

    QString m_cpuUsage = "0%";
    QString m_ramUsage = "0 MB";
    QString m_vramUsage = "0 MB";
    QString m_appMemory = "0 MB";

    QString m_currentEngine = "llama.cpp";
    QString m_enginePath = "/usr/bin/llama-server"; // <-- DEVUELTO
    QStringList m_availableEngines = {"llama.cpp", "Ollama", "vLLM", "LM Studio"};

    QString m_currentModel = "Ninguno";
    QVariantList m_localModels;
    QStringList m_chatMessages;
    bool m_isGenerating = false;
};
