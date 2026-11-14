#include "AgentEngine.h"
#include "tools/FileSystemTools.h"
#include "tools/TerminalTool.h"
#include "tools/WebTools.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QDir>
#include <QUrl>
#include <QDate>

AgentEngine::AgentEngine(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_memory(4096)
    , m_workspaceDir(QDir::currentPath())
{
    registerFileSystemTools(m_toolRegistry);
    registerTerminalTools(m_toolRegistry);
    registerWebTools(m_toolRegistry);
    updateSystemPromptWithWorkspace();
}

void AgentEngine::setWorkspaceDir(const QString &dirPath) {
    QDir dir(dirPath);
    if (!dir.exists()) return;
    m_workspaceDir = dir.absolutePath();
    QDir::setCurrent(m_workspaceDir);
    updateSystemPromptWithWorkspace();
}

void AgentEngine::updateSystemPromptWithWorkspace() {
    const QString today = QDate::currentDate().toString(Qt::ISODate);
    m_memory.setSystemPrompt(QStringLiteral(
        "Eres FronterMind, un ingeniero de software e investigador autónomo ejecutándose localmente. "
        "Fecha actual del sistema: %1. "
        "Directorio raíz del proyecto activo (Workspace): '%2'. "
        "Cuando el usuario te pida inspeccionar, leer, crear o modificar archivos, ejecutar comandos o buscar en internet, "
        "utiliza tus herramientas (list_directory, read_file, write_file, edit_file, run_shell_command, web_search, http_fetch). "
        "Al usar web_search sobre eventos o versiones recientes, incluye el año actual (%3) en la consulta. "
        "Responde en Markdown estructurado y jamás utilices emojis.")
        .arg(today, m_workspaceDir, QString::number(QDate::currentDate().year())));
}

QString AgentEngine::summarizeToolArgs(const QString &toolName, const QString &argsJson) {
    const QJsonObject obj = QJsonDocument::fromJson(argsJson.toUtf8()).object();
    if (obj.contains("path"))    return obj["path"].toString();
    if (obj.contains("command")) return obj["command"].toString();
    if (obj.contains("query"))   return QStringLiteral("\"%1\"").arg(obj["query"].toString());
    if (obj.contains("url"))     return obj["url"].toString();
    return argsJson.simplified().left(80);
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

bool AgentEngine::addMcpServer(const QString &name, const QString &command, const QStringList &args, QString &errorOut) {
    auto *client = new McpClient(name, this);
    if (!client->connectToServer(command, args, errorOut)) {
        delete client;
        return false;
    }
    for (const McpToolInfo &info : client->discoveredTools()) {
        m_toolRegistry.registerTool(std::make_unique<McpBridgeTool>(client, info));
    }
    m_mcpClients.append(client);
    return true;
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
    if (text.trimmed().isEmpty()) return;

    emit startedGenerating();
    m_memory.addUserMessage(text);
    emit tokensUpdated(m_memory.usedTokens(), 0, m_memory.usedTokens());

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
    rootObj["temperature"] = 0.4;
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
                    if (fnObj["arguments"].isString()) {
                        call.argumentsJson = fnObj["arguments"].toString();
                    } else if (fnObj["arguments"].isObject()) {
                        call.argumentsJson = QString::fromUtf8(
                            QJsonDocument(fnObj["arguments"].toObject()).toJson(QJsonDocument::Compact)
                        );
                    }
                    parsedCalls.append(call);
                }

                m_memory.addAssistantMessage(stripThinkingBlocks(content), parsedCalls);

                for (const ToolCallData &call : parsedCalls) {
                    const QString targetSummary = summarizeToolArgs(call.name, call.argumentsJson);
                    const ToolResult res = m_toolRegistry.executeTool(call.name, call.argumentsJson);

                    // Emitir evento transparente para que el usuario vea en el chat qué hizo el agente
                    QString firstLinePreview = res.output.section('\n', 0, 0).trimmed().left(90);
                    emit toolActionLogged(call.name, targetSummary, firstLinePreview, res.isError);

                    m_memory.addToolResultMessage(call.id, call.name, res.output);
                }

                performInferenceStep(remainingSteps - 1);
                return;
            }
        }

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
