#pragma once
#include "ChatMessage.h"
#include <QList>
#include <QJsonArray>
#include <QString>

class ConversationMemory {
public:
    explicit ConversationMemory(int maxContextTokens = 4096);

    // Configuración de contexto y System Prompt
    void setSystemPrompt(const QString &prompt);
    QString systemPrompt() const { return m_systemPrompt; }

    void setMaxContextTokens(int maxTokens);
    int maxContextTokens() const { return m_maxContextTokens; }

    void setReservedOutputTokens(int reservedTokens);
    int reservedOutputTokens() const { return m_reservedOutputTokens; }

    // Registro de mensajes en la memoria
    void addUserMessage(const QString &content);
    void addAssistantMessage(const QString &content, const QList<ToolCallData> &toolCalls = {});
    void addToolResultMessage(const QString &toolCallId, const QString &toolName, const QString &resultContent);

    // Sincronización con el conteo exacto que reporta llama-server (prompt_tokens + completion_tokens)
    void updateActualTokenUsage(int promptTokens, int completionTokens);

    // Limpieza de sesión
    void clear();

    // Estadísticas y exportación para el motor LLM
    int usedTokens() const;
    bool isEmpty() const { return m_messages.isEmpty(); }
    const QList<ChatMessage>& messages() const { return m_messages; }

    // Construye el array JSON "messages" aplicando ventana deslizante para no exceder maxContextTokens
    QJsonArray buildApiMessagesPayload() const;

private:
    void pruneIfNeeded();

    QString m_systemPrompt;
    int m_maxContextTokens = 4096;
    int m_reservedOutputTokens = 1024;
    int m_lastReportedTotalTokens = 0;
    QList<ChatMessage> m_messages;
};
