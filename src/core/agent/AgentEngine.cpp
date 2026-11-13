#include "AgentEngine.h"
#include "tools/FileSystemTools.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QUrl>
#include <QDebug>
#include "tools/TerminalTool.h"
#include "tools/WebTools.h"

AgentEngine::AgentEngine(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_memory(4096)
{
    // Registrar herramientas de Sistema de Archivos al iniciar el motor
    registerFileSystemTools(m_toolRegistry);
    registerFileSystemTools(m_toolRegistry);
    registerTerminalTools(m_toolRegistry);
    registerFileSystemTools(m_toolRegistry);
    registerTerminalTools(m_toolRegistry);
    registerWebTools(m_toolRegistry);
}

void AgentEngine::clearMemory() {
    m_memory.clear();
    emit tokensUpdated(0, 0, 0);
}

void AgentEngine::setMaxContextTokens(int maxTokens) {
    m_memory.setMaxContextTokens(maxTokens);
}

void AgentEngine::setSystemPrompt(const QString &prompt) {
    m_memory.setSystemPrompt(prompt);
}

QString AgentEngine::stripThinkingBlocks(const QString &text) {
    QString cleaned = text;
    static const QRegularExpression thinkRegex(
        R"(<think>[\s\S]*?</think>\s*)",
        QRegularExpression::CaseInsensitiveOption
    );
    cleaned.remove(thinkRegex);
    return cleaned.trimmed();
}

void AgentEngine::sendMessage(const QString &modelName, const QString &text) {
    Q_UNUSED(modelName);

    if (text.trimmed().isEmpty()) {
        return;
    }

    emit startedGenerating();
    m_memory.addUserMessage(text);
    emit tokensUpdated(m_memory.usedTokens(), 0, m_memory.usedTokens());

    // Iniciar el ciclo ReAct (máximo 8 pasos consecutivos de herramientas por mensaje)
    performInferenceStep(8);
}

void AgentEngine::performInferenceStep(int remainingSteps) {
    if (remainingSteps <= 0) {
        emit errorOccurred(QStringLiteral("Se alcanzó el límite máximo de pasos de herramientas por turno."));
        emit finishedGenerating();
        return;
    }

    QUrl url(m_serverUrl);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    QJsonObject rootObj;
    rootObj["model"] = QStringLiteral("local");
    rootObj["messages"] = m_memory.buildApiMessagesPayload();
    rootObj["temperature"] = 0.4; // Temperatura más precisa para código y uso de herramientas
    rootObj["stream"] = false;

    if (!m_toolRegistry.isEmpty()) {
        rootObj["tools"] = m_toolRegistry.buildToolsJson();
        rootObj["tool_choice"] = QStringLiteral("auto");
    }

    QJsonDocument doc(rootObj);
    QNetworkReply *reply = m_networkManager->post(request, doc.toJson(QJsonDocument::Compact));

    connect(reply, &QNetworkReply::finished, this, [this, reply, remainingSteps]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred(reply->errorString());
            emit finishedGenerating();
            return;
        }

        const QByteArray responseData = reply->readAll();
        const QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
        const QJsonObject jsonObj = jsonResponse.object();

        if (jsonObj.contains("usage")) {
            const QJsonObject usage = jsonObj["usage"].toObject();
            const int promptTok = usage["prompt_tokens"].toInt();
            const int compTok   = usage["completion_tokens"].toInt();
            const int totalTok  = usage["total_tokens"].toInt();

            m_memory.updateActualTokenUsage(promptTok, compTok);
            emit tokensUpdated(promptTok, compTok, totalTok);
        }

        if (!jsonObj.contains("choices")) {
            emit errorOccurred(QStringLiteral("Respuesta inválida del servidor (sin campo 'choices')."));
            emit finishedGenerating();
            return;
        }

        const QJsonArray choices = jsonObj["choices"].toArray();
        if (choices.isEmpty()) {
            emit errorOccurred(QStringLiteral("El servidor devolvió 'choices' vacío."));
            emit finishedGenerating();
            return;
        }

        const QJsonObject choice = choices[0].toObject();
        const QJsonObject message = choice["message"].toObject();
        QString content = message["content"].toString();

        // 1. Verificar si el modelo solicitó ejecutar herramientas (Tool Calls)
        if (message.contains("tool_calls") && message["tool_calls"].isArray()) {
            const QJsonArray toolCallsJson = message["tool_calls"].toArray();
            if (!toolCallsJson.isEmpty()) {
                QList<ToolCallData> parsedCalls;
                for (const QJsonValue &tcVal : toolCallsJson) {
                    const QJsonObject tcObj = tcVal.toObject();
                    const QJsonObject fnObj = tcObj["function"].toObject();

                    ToolCallData call;
                    call.id = tcObj["id"].toString(QStringLiteral("call_1"));
                    call.name = fnObj["name"].toString();
                    // arguments puede venir como string JSON o como objeto JSON
                    if (fnObj["arguments"].isString()) {
                        call.argumentsJson = fnObj["arguments"].toString();
                    } else if (fnObj["arguments"].isObject()) {
                        call.argumentsJson = QString::fromUtf8(
                            QJsonDocument(fnObj["arguments"].toObject()).toJson(QJsonDocument::Compact)
                        );
                    }
                    parsedCalls.append(call);
                }

                // Guardar la petición de herramientas del asistente en la memoria
                m_memory.addAssistantMessage(stripThinkingBlocks(content), parsedCalls);

                // Ejecutar cada herramienta solicitada y guardar su resultado en la memoria
                for (const ToolCallData &call : parsedCalls) {
                    emit toolExecutionStarted(call.name);
                    const ToolResult res = m_toolRegistry.executeTool(call.name, call.argumentsJson);
                    m_memory.addToolResultMessage(call.id, call.name, res.output);
                }

                // Continuar el bucle ReAct enviando los resultados de vuelta al modelo
                performInferenceStep(remainingSteps - 1);
                return;
            }
        }

        // 2. Si no hubo llamadas a herramientas, es la respuesta final al usuario
        if (content.isEmpty() && message.contains("reasoning_content")) {
            content = message["reasoning_content"].toString();
        }

        if (content.isEmpty()) {
            emit errorOccurred(QStringLiteral("El modelo devolvió una respuesta vacía."));
        } else {
            const QString cleanForMemory = stripThinkingBlocks(content);
            m_memory.addAssistantMessage(cleanForMemory.isEmpty() ? content : cleanForMemory);
            emit messageReceived(content);
        }

        emit finishedGenerating();
    });
}
