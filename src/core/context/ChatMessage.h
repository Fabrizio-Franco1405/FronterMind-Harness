#pragma once
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>

enum class MessageRole {
    System,
    User,
    Assistant,
    Tool
};

struct ToolCallData {
    QString id;
    QString name;
    QString argumentsJson;

    QJsonObject toJson() const {
        QJsonObject fnObj;
        fnObj["name"] = name;
        fnObj["arguments"] = argumentsJson;

        QJsonObject callObj;
        callObj["id"] = id;
        callObj["type"] = "function";
        callObj["function"] = fnObj;
        return callObj;
    }
};

struct ChatMessage {
    MessageRole role = MessageRole::User;
    QString content;
    QString toolCallId;                // Usado cuando role == MessageRole::Tool
    QString toolName;                  // Nombre de la herramienta ejecutada
    QList<ToolCallData> toolCalls;     // Usado cuando el Assistant invoca herramientas
    int estimatedTokens = 0;
    qint64 timestamp = 0;

    ChatMessage() = default;

    ChatMessage(MessageRole r, const QString &text)
        : role(r)
        , content(text)
        , estimatedTokens(estimateTokenCount(text))
        , timestamp(QDateTime::currentMSecsSinceEpoch())
    {}

    static int estimateTokenCount(const QString &text) {
        if (text.isEmpty()) return 0;
        // Regla heurística rápida para modelos BPE (Qwen/Llama): ~3.6 caracteres por token + 4 tokens de sobrecarga ChatML
        return qMax(1, (text.length() * 10) / 36) + 4;
    }

    static QString roleToString(MessageRole r) {
        switch (r) {
            case MessageRole::System:    return QStringLiteral("system");
            case MessageRole::User:      return QStringLiteral("user");
            case MessageRole::Assistant: return QStringLiteral("assistant");
            case MessageRole::Tool:      return QStringLiteral("tool");
        }
        return QStringLiteral("user");
    }

    QJsonObject toApiJson() const {
        QJsonObject obj;
        obj["role"] = roleToString(role);
        obj["content"] = content;

        if (role == MessageRole::Tool && !toolCallId.isEmpty()) {
            obj["tool_call_id"] = toolCallId;
            if (!toolName.isEmpty()) {
                obj["name"] = toolName;
            }
        }

        if (role == MessageRole::Assistant && !toolCalls.isEmpty()) {
            QJsonArray callsArray;
            for (const auto &tc : toolCalls) {
                callsArray.append(tc.toJson());
            }
            obj["tool_calls"] = callsArray;
        }

        return obj;
    }
};
