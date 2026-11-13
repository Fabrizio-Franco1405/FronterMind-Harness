#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "context/ConversationMemory.h"
#include "tools/ToolRegistry.h"
#include "mcp/McpClient.h"

class AgentEngine : public QObject {
    Q_OBJECT
public:
    explicit AgentEngine(QObject *parent = nullptr);

    void sendMessage(const QString &modelName, const QString &text);

    void clearMemory();
    void setMaxContextTokens(int maxTokens);
    int maxContextTokens() const { return m_memory.maxContextTokens(); }
    int usedTokens() const { return m_memory.usedTokens(); }

    void setSystemPrompt(const QString &prompt);
    QString systemPrompt() const { return m_memory.systemPrompt(); }

    void setServerUrl(const QString &url) { m_serverUrl = url; }
    QString serverUrl() const { return m_serverUrl; }

    ToolRegistry& toolRegistry() { return m_toolRegistry; }

    bool addMcpServer(const QString &name, const QString &command, const QStringList &args, QString &errorOut);
    QVariantList activeToolsMetadata() const { return m_toolRegistry.getRegisteredToolsMetadata(); }

signals:
    void messageReceived(const QString &response);
    void errorOccurred(const QString &error);
    void startedGenerating();
    void finishedGenerating();
    void toolExecutionStarted(const QString &toolName);
    void tokensUpdated(int promptTokens, int completionTokens, int totalTokens);

private:
    void performInferenceStep(int remainingSteps);
    static QString stripThinkingBlocks(const QString &text);

    QNetworkAccessManager *m_networkManager = nullptr;
    ConversationMemory m_memory;
    ToolRegistry m_toolRegistry;
    QString m_serverUrl = QStringLiteral("http://127.0.0.1:8080/v1/chat/completions");
    QList<McpClient*> m_mcpClients;
};
