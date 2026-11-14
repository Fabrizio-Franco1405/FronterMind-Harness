#include "ConversationMemory.h"
#include <QtGlobal>

ConversationMemory::ConversationMemory(int maxContextTokens)
    : m_maxContextTokens(maxContextTokens)
    , m_systemPrompt(QStringLiteral(
        "Eres FronterMind, un asistente e ingeniero de software local experto en C++, Qt6, QML, Linux y arquitectura de sistemas. "
        "Responde de forma clara, directa y estructurada usando formato Markdown."))
{
}

void ConversationMemory::setSystemPrompt(const QString &prompt) {
    m_systemPrompt = prompt.trimmed();
}

void ConversationMemory::setMaxContextTokens(int maxTokens) {
    m_maxContextTokens = qMax(512, maxTokens);
    // Reservamos el 25% del contexto para la respuesta, con tope de 2048 tokens
    m_reservedOutputTokens = qBound(256, m_maxContextTokens / 4, 2048);
    pruneIfNeeded();
}

void ConversationMemory::setReservedOutputTokens(int reservedTokens) {
    m_reservedOutputTokens = qBound(128, reservedTokens, m_maxContextTokens / 2);
    pruneIfNeeded();
}

void ConversationMemory::addUserMessage(const QString &content) {
    m_messages.append(ChatMessage(MessageRole::User, content));
    m_lastReportedTotalTokens = 0; // Forzar recálculo hasta que el servidor responda
    pruneIfNeeded();
}

void ConversationMemory::addAssistantMessage(const QString &content, const QList<ToolCallData> &toolCalls) {
    ChatMessage msg(MessageRole::Assistant, content);
    msg.toolCalls = toolCalls;
    m_messages.append(msg);
    pruneIfNeeded();
}

void ConversationMemory::addToolResultMessage(const QString &toolCallId, const QString &toolName, const QString &resultContent) {
    ChatMessage msg(MessageRole::Tool, resultContent);
    msg.toolCallId = toolCallId;
    msg.toolName = toolName;
    m_messages.append(msg);
    m_lastReportedTotalTokens = 0;
    pruneIfNeeded();
}

void ConversationMemory::updateActualTokenUsage(int promptTokens, int completionTokens) {
    if (promptTokens > 0 || completionTokens > 0) {
        m_lastReportedTotalTokens = promptTokens + completionTokens;
    }
}

void ConversationMemory::clear() {
    m_messages.clear();
    m_lastReportedTotalTokens = 0;
}

int ConversationMemory::usedTokens() const {
    if (m_lastReportedTotalTokens > 0) {
        return m_lastReportedTotalTokens;
    }

    int total = m_systemPrompt.isEmpty() ? 0 : ChatMessage::estimateTokenCount(m_systemPrompt);
    for (const auto &msg : m_messages) {
        total += msg.estimatedTokens;
    }
    return total;
}

void ConversationMemory::pruneIfNeeded() {
    const int systemTokens = m_systemPrompt.isEmpty() ? 0 : ChatMessage::estimateTokenCount(m_systemPrompt);
    const int maxHistoryBudget = qMax(256, m_maxContextTokens - m_reservedOutputTokens - systemTokens);

    auto calculateHistoryTokens = [this]() {
        int sum = 0;
        for (const auto &m : m_messages) {
            sum += m.estimatedTokens;
        }
        return sum;
    };

    // Si superamos el presupuesto de tokens, eliminamos desde el mensaje más antiguo,
    // pero conservamos siempre al menos el último mensaje enviado por el usuario.
    while (m_messages.size() > 1 && calculateHistoryTokens() > maxHistoryBudget) {
        m_messages.removeFirst();
        // Si al borrar un mensaje quedó huérfano un resultado de herramienta al inicio, también lo limpiamos
        while (m_messages.size() > 1 && m_messages.first().role == MessageRole::Tool) {
            m_messages.removeFirst();
        }
    }
}

QJsonArray ConversationMemory::buildApiMessagesPayload() const {
    QJsonArray array;

    if (!m_systemPrompt.isEmpty()) {
        ChatMessage sysMsg(MessageRole::System, m_systemPrompt);
        array.append(sysMsg.toApiJson());
    }

    for (const auto &msg : m_messages) {
        array.append(msg.toApiJson());
    }

    return array;
}
