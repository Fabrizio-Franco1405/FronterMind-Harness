// ============================================================================
// SECCIÓN 1: CABECERAS Y DEPENDENCIAS DEL MOTOR AGÉNTICO
// ============================================================================
#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>
#include <QJsonArray>
#include <QList>

#include "context/ConversationMemory.h"
#include "tools/ToolRegistry.h"
#include "mcp/McpClient.h"

// ============================================================================
// SECCIÓN 2: CLASE AGENTENGINE (ORQUESTADOR DE INFERENCIA, STREAMING Y TOOLS)
// ============================================================================
class AgentEngine : public QObject {
    Q_OBJECT

public:
    explicit AgentEngine(QObject *parent = nullptr);
    ~AgentEngine() override = default;

    // --- 2.1: Acciones Principales de Inferencia ---
    void sendMessage(const QString &modelName, const QString &text);
    void stopGeneration();
    void clearMemory();

    // --- 2.2: Configuración de Contexto y Espacio de Trabajo ---
    void setMaxContextTokens(int maxTokens);
    void setWorkspaceDir(const QString &dirPath);
    QString workspaceDir() const { return m_workspaceDir; }

    void setSystemPrompt(const QString &prompt);
    QString systemPrompt() const { return m_memory.systemPrompt(); }

    // --- 2.3: Configuración del Servidor y Parámetros de Inferencia (v0.2.0) ---
    void setServerUrl(const QString &url) { m_serverUrl = url; }
    QString serverUrl() const { return m_serverUrl; }

    void setTemperature(double temp) { m_temperature = qBound(0.0, temp, 2.0); }
    double temperature() const { return m_temperature; }

    void setTopP(double topP) { m_topP = qBound(0.05, topP, 1.0); }
    double topP() const { return m_topP; }

    // --- 2.4: Gestión de Herramientas y Servidores MCP ---
    bool addMcpServer(const QString &name, const QString &command, const QStringList &args, QString &errorOut);
    QVariantList activeToolsMetadata() const { return m_toolRegistry.getRegisteredToolsMetadata(); }
    ToolRegistry& toolRegistry() { return m_toolRegistry; }

signals:
    // --- 2.5: Señales Reactivas de Ciclo de Vida y Streaming SSE ---
    void startedGenerating();
    void streamChunkReceived(const QString &partialContent);
    void messageReceived(const QString &response);
    void errorOccurred(const QString &error);
    void finishedGenerating();
    void tokensUpdated(int promptTokens, int completionTokens, int totalTokens);
    void toolActionLogged(const QString &toolName, const QString &targetSummary, const QString &resultPreview, bool isError);

private:
    // --- 2.6: Métodos Internos del Ciclo Agéntico ---
    void performInferenceStep(int remainingSteps);
    void updateSystemPromptWithWorkspace();
    static QString stripThinkingBlocks(const QString &text);
    static QString summarizeToolArgs(const QString &toolName, const QString &argsJson);
    static QList<ToolCallData> extractXmlToolCalls(QString &contentInOut);

    // --- 2.7: Estado Interno ---
    QNetworkAccessManager *m_networkManager = nullptr;
    QPointer<QNetworkReply> m_activeReply = nullptr;
    QByteArray m_sseBuffer;
    QString m_streamedContent;
    QJsonArray m_streamedToolCalls;

    ConversationMemory m_memory;
    ToolRegistry m_toolRegistry;
    QList<McpClient*> m_mcpClients;

    QString m_workspaceDir;
    QString m_serverUrl = QStringLiteral("http://127.0.0.1:8080/v1/chat/completions");
    double m_temperature = 0.4;
    double m_topP = 0.95;
};
