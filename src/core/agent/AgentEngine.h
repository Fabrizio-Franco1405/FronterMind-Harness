#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "context/ConversationMemory.h"

class AgentEngine : public QObject {
    Q_OBJECT
public:
    explicit AgentEngine(QObject *parent = nullptr);

    // Envío de mensajes con memoria de conversación integrada
    void sendMessage(const QString &modelName, const QString &text);

    // Gestión de memoria y contexto
    void clearMemory();
    void setMaxContextTokens(int maxTokens);
    int maxContextTokens() const { return m_memory.maxContextTokens(); }
    int usedTokens() const { return m_memory.usedTokens(); }

    void setSystemPrompt(const QString &prompt);
    QString systemPrompt() const { return m_memory.systemPrompt(); }

    void setServerUrl(const QString &url) { m_serverUrl = url; }
    QString serverUrl() const { return m_serverUrl; }

signals:
    void messageReceived(const QString &response);
    void errorOccurred(const QString &error);
    void startedGenerating();
    void finishedGenerating();
    void tokensUpdated(int promptTokens, int completionTokens, int totalTokens);

private:
    static QString stripThinkingBlocks(const QString &text);

    QNetworkAccessManager *m_networkManager = nullptr;
    ConversationMemory m_memory;
    QString m_serverUrl = QStringLiteral("http://127.0.0.1:8080/v1/chat/completions");
};
