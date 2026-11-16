// ============================================================================
// SECCIÓN 1: CABECERAS E INCLUSIONES DE HERRAMIENTAS NATIVAS
// ============================================================================
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

// ============================================================================
// SECCIÓN 2: INICIALIZACIÓN DEL MOTOR Y REGISTRO DE HERRAMIENTAS C++
// ============================================================================
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

// ============================================================================
// SECCIÓN 3: SYSTEM PROMPT CON PROTOCOLO DUAL DE HERRAMIENTAS (OPENAI + XML)
// ============================================================================
void AgentEngine::updateSystemPromptWithWorkspace() {
    const QString today = QDate::currentDate().toString(Qt::ISODate);
    const QString currentYear = QString::number(QDate::currentDate().year());

    m_memory.setSystemPrompt(QStringLiteral(
        "Eres FronterMind, un ingeniero de software e investigador autónomo ejecutándose localmente.\n"
        "Fecha actual del sistema: %1.\n"
        "Directorio raíz del proyecto activo (Workspace): '%2'.\n\n"
        "REGLAS ESTRICTAS DE HERRAMIENTAS:\n"
        "1. TIENES ACCESO REAL Y DIRECTO al sistema de archivos y a internet mediante tus herramientas registradas: "
        "list_directory, read_file, write_file, edit_file, run_shell_command, web_search, http_fetch.\n"
        "2. PROHIBIDO decir que no tienes acceso a internet o al disco duro, y PROHIBIDO inventar nombres de archivos.\n"
        "3. Al usar web_search sobre versiones o eventos recientes, incluye siempre el año %3 en tu búsqueda.\n"
        "4. Si tu plantilla soporta tool_calls nativos, úsalos. Si no, puedes invocar cualquier herramienta emitiendo "
        "exactamente este bloque XML:\n"
        "<tool_call>\n{\"name\": \"nombre_herramienta\", \"arguments\": {\"param\": \"valor\"}}\n</tool_call>\n"
        "5. Responde en Markdown limpio y jamás utilices emojis.")
        .arg(today, m_workspaceDir, currentYear));
}

// ============================================================================
// SECCIÓN 4: PARSER UNIVERSAL DE <tool_call> Y LIMPIEZA DE PENSAMIENTO
// ============================================================================
QList<ToolCallData> AgentEngine::extractXmlToolCalls(QString &contentInOut) {
    QList<ToolCallData> calls;

    // Estándar Hermes / Qwen / DeepSeek: <tool_call>{"name": "...", "arguments": {...}}</tool_call>
    static const QRegularExpression xmlToolRegex(
        R"(<tool_call>\s*(\{[\s\S]*?\})\s*</tool_call>)",
        QRegularExpression::CaseInsensitiveOption
    );

    auto it = xmlToolRegex.globalMatch(contentInOut);
    int idx = 1;
    while (it.hasNext()) {
        auto match = it.next();
        QJsonParseError err;
        const QJsonDocument doc = QJsonDocument::fromJson(match.captured(1).toUtf8(), &err);
        if (err.error == QJsonParseError::NoError && doc.isObject()) {
            const QJsonObject obj = doc.object();
            const QString fnName = obj["name"].toString().trimmed();
            if (!fnName.isEmpty()) {
                ToolCallData call;
                call.id = QStringLiteral("call_xml_%1").arg(idx++);
                call.name = fnName;
                if (obj["arguments"].isObject()) {
                    call.argumentsJson = QString::fromUtf8(
                        QJsonDocument(obj["arguments"].toObject()).toJson(QJsonDocument::Compact)
                    );
                } else if (obj["arguments"].isString()) {
                    call.argumentsJson = obj["arguments"].toString();
                } else {
                    call.argumentsJson = QStringLiteral("{}");
                }
                calls.append(call);
            }
        }
    }

    if (!calls.isEmpty()) {
        contentInOut.remove(xmlToolRegex);
        contentInOut = contentInOut.trimmed();
    }

    return calls;
}

QString AgentEngine::summarizeToolArgs(const QString &toolName, const QString &argsJson) {
    Q_UNUSED(toolName);
    const QJsonObject obj = QJsonDocument::fromJson(argsJson.toUtf8()).object();
    if (obj.contains("path"))    return obj["path"].toString();
    if (obj.contains("command")) return obj["command"].toString();
    if (obj.contains("query"))   return obj["query"].toString();
    if (obj.contains("url"))     return obj["url"].toString();
    return argsJson.simplified().left(80);
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

// ============================================================================
// SECCIÓN 5: GESTIÓN DE MEMORIA, MCP Y CANCELACIÓN DE GENERACIÓN
// ============================================================================
void AgentEngine::clearMemory() {
    stopGeneration();
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

void AgentEngine::stopGeneration() {
    if (m_activeReply) {
        m_activeReply->abort();
        m_activeReply->deleteLater();
        m_activeReply = nullptr;
    }
    emit finishedGenerating();
}

void AgentEngine::sendMessage(const QString &modelName, const QString &text) {
    Q_UNUSED(modelName);
    if (text.trimmed().isEmpty()) return;

    emit startedGenerating();
    m_memory.addUserMessage(text);
    emit tokensUpdated(m_memory.usedTokens(), 0, m_memory.usedTokens());

    performInferenceStep(8);
}

// ============================================================================
// SECCIÓN 6: BUCLE DE INFERENCIA CON STREAMING SSE EN TIEMPO REAL (LATENCIA 0)
// ============================================================================
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
    rootObj["temperature"] = m_temperature;
    rootObj["top_p"] = m_topP;
    rootObj["max_tokens"] = 1536;
    rootObj["repeat_penalty"] = 1.12;
    rootObj["stream"] = true;

    if (!m_toolRegistry.isEmpty()) {
        rootObj["tools"] = m_toolRegistry.buildToolsJson();
        rootObj["tool_choice"] = QStringLiteral("auto");
    }

    m_sseBuffer.clear();
    m_streamedContent.clear();
    m_streamedToolCalls = QJsonArray();

    QJsonDocument doc(rootObj);
    QNetworkReply *reply = m_networkManager->post(request, doc.toJson(QJsonDocument::Compact));
    m_activeReply = reply;

    // --- 6.1: Lectura en tiempo real de fragmentos Server-Sent Events (SSE) ---
    connect(reply, &QNetworkReply::readyRead, this, [this, reply]() {
        if (m_activeReply != reply) return;
        m_sseBuffer.append(reply->readAll());

        int newlineIdx;
        while ((newlineIdx = m_sseBuffer.indexOf('\n')) != -1) {
            QByteArray line = m_sseBuffer.left(newlineIdx).trimmed();
            m_sseBuffer.remove(0, newlineIdx + 1);

            if (line.isEmpty() || !line.startsWith("data:")) continue;
            QByteArray dataPayload = line.mid(5).trimmed();
            if (dataPayload == "[DONE]") continue;

            QJsonParseError parseErr;
            const QJsonDocument chunkDoc = QJsonDocument::fromJson(dataPayload, &parseErr);
            if (parseErr.error != QJsonParseError::NoError || !chunkDoc.isObject()) continue;

            const QJsonObject chunkObj = chunkDoc.object();

            // Actualizar contador de tokens si viene incluido en el chunk SSE
            if (chunkObj.contains("usage") && chunkObj["usage"].isObject()) {
                const QJsonObject usage = chunkObj["usage"].toObject();
                const int promptTok = usage["prompt_tokens"].toInt();
                const int compTok   = usage["completion_tokens"].toInt();
                const int totalTok  = usage["total_tokens"].toInt();
                if (totalTok > 0) {
                    m_memory.updateActualTokenUsage(promptTok, compTok);
                    emit tokensUpdated(promptTok, compTok, totalTok);
                }
            }

            const QJsonArray choices = chunkObj["choices"].toArray();
            if (choices.isEmpty()) continue;

            const QJsonObject choice = choices[0].toObject();
            const QJsonObject delta = choice["delta"].toObject();

            // Acumular razonamiento (<think>) o contenido final token por token
            const QString reasoningDelta = delta["reasoning_content"].toString();
            const QString contentDelta   = delta["content"].toString();

            if (!reasoningDelta.isEmpty()) {
                if (!m_streamedContent.startsWith("<think>")) {
                    m_streamedContent.prepend("<think>");
                }
                m_streamedContent += reasoningDelta;
            }
            if (!contentDelta.isEmpty()) {
                if (m_streamedContent.startsWith("<think>") && !m_streamedContent.contains("</think>")) {
                    m_streamedContent += "</think>\n\n";
                }
                m_streamedContent += contentDelta;
            }

            // Acumular tool_calls nativos de OpenAI transmitidos por partes (deltas)
            if (delta.contains("tool_calls") && delta["tool_calls"].isArray()) {
                for (const QJsonValue &tcVal : delta["tool_calls"].toArray()) {
                    const QJsonObject tcDelta = tcVal.toObject();
                    const int index = tcDelta["index"].toInt(0);

                    while (m_streamedToolCalls.size() <= index) {
                        QJsonObject emptyTc;
                        emptyTc["id"] = QStringLiteral("call_%1").arg(m_streamedToolCalls.size() + 1);
                        emptyTc["type"] = QStringLiteral("function");
                        emptyTc["function"] = QJsonObject{{"name", ""}, {"arguments", ""}};
                        m_streamedToolCalls.append(emptyTc);
                    }

                    QJsonObject currentTc = m_streamedToolCalls[index].toObject();
                    if (tcDelta.contains("id") && !tcDelta["id"].toString().isEmpty()) {
                        currentTc["id"] = tcDelta["id"].toString();
                    }
                    if (tcDelta.contains("function") && tcDelta["function"].isObject()) {
                        const QJsonObject fnDelta = tcDelta["function"].toObject();
                        QJsonObject currentFn = currentTc["function"].toObject();
                        if (fnDelta.contains("name")) {
                            currentFn["name"] = currentFn["name"].toString() + fnDelta["name"].toString();
                        }
                        if (fnDelta.contains("arguments")) {
                            currentFn["arguments"] = currentFn["arguments"].toString() + fnDelta["arguments"].toString();
                        }
                        currentTc["function"] = currentFn;
                    }
                    m_streamedToolCalls[index] = currentTc;
                }
            }

            // Emitir el avance en vivo a la interfaz si no hay tool_calls silenciosos en curso
            if (m_streamedToolCalls.isEmpty() && !m_streamedContent.isEmpty() && !m_streamedContent.contains("<tool_call>")) {
                emit streamChunkReceived(m_streamedContent);
            }
        }
    });

    // --- 6.2: Cierre del Stream SSE o Fallback JSON Completo ---
    connect(reply, &QNetworkReply::finished, this, [this, reply, remainingSteps]() {
        if (m_activeReply == reply) {
            m_activeReply = nullptr;
        }
        reply->deleteLater();

        if (reply->error() == QNetworkReply::OperationCanceledError) {
            emit finishedGenerating();
            return;
        }

        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred(reply->errorString());
            emit finishedGenerating();
            return;
        }

        // Si el servidor devolvió un JSON normal sin prefijos "data:" (fallback no-SSE)
        if (m_streamedContent.isEmpty() && m_streamedToolCalls.isEmpty() && !m_sseBuffer.trimmed().isEmpty()) {
            const QJsonDocument fullDoc = QJsonDocument::fromJson(m_sseBuffer);
            if (fullDoc.isObject()) {
                const QJsonObject jsonObj = fullDoc.object();
                if (jsonObj.contains("usage")) {
                    const QJsonObject usage = jsonObj["usage"].toObject();
                    m_memory.updateActualTokenUsage(usage["prompt_tokens"].toInt(), usage["completion_tokens"].toInt());
                    emit tokensUpdated(usage["prompt_tokens"].toInt(), usage["completion_tokens"].toInt(), usage["total_tokens"].toInt());
                }
                const QJsonArray choices = jsonObj["choices"].toArray();
                if (!choices.isEmpty()) {
                    const QJsonObject msg = choices[0].toObject()["message"].toObject();
                    m_streamedContent = msg["content"].toString();
                    if (m_streamedContent.isEmpty() && msg.contains("reasoning_content")) {
                        m_streamedContent = msg["reasoning_content"].toString();
                    }
                    if (msg.contains("tool_calls") && msg["tool_calls"].isArray()) {
                        m_streamedToolCalls = msg["tool_calls"].toArray();
                    }
                }
            }
        }

        // 1. Recolectar llamadas a herramientas nativas (OpenAI tool_calls)
        QList<ToolCallData> parsedCalls;
        for (const QJsonValue &tcVal : m_streamedToolCalls) {
            const QJsonObject tcObj = tcVal.toObject();
            const QJsonObject fnObj = tcObj["function"].toObject();
            const QString fnName = fnObj["name"].toString().trimmed();
            if (fnName.isEmpty()) continue;

            ToolCallData call;
            call.id = tcObj["id"].toString(QStringLiteral("call_1"));
            call.name = fnName;
            if (fnObj["arguments"].isString()) {
                call.argumentsJson = fnObj["arguments"].toString();
            } else if (fnObj["arguments"].isObject()) {
                call.argumentsJson = QString::fromUtf8(
                    QJsonDocument(fnObj["arguments"].toObject()).toJson(QJsonDocument::Compact)
                );
            }
            parsedCalls.append(call);
        }

        // 2. Si no hubo tool_calls nativos, buscar etiquetas <tool_call> en texto (DeepSeek-R1 / Qwen)
        if (parsedCalls.isEmpty() && m_streamedContent.contains("<tool_call>", Qt::CaseInsensitive)) {
            parsedCalls = extractXmlToolCalls(m_streamedContent);
        }

        // 3. Si el agente solicitó ejecutar herramientas, ejecutarlas y continuar el ciclo
        if (!parsedCalls.isEmpty()) {
            m_memory.addAssistantMessage(stripThinkingBlocks(m_streamedContent), parsedCalls);

            for (const ToolCallData &call : parsedCalls) {
                const QString targetSummary = summarizeToolArgs(call.name, call.argumentsJson);
                const ToolResult res = m_toolRegistry.executeTool(call.name, call.argumentsJson);

                const QString firstLinePreview = res.output.section('\n', 0, 0).trimmed().left(90);
                emit toolActionLogged(call.name, targetSummary, firstLinePreview, res.isError);

                m_memory.addToolResultMessage(call.id, call.name, res.output);
            }

            performInferenceStep(remainingSteps - 1);
            return;
        }

        // 4. Respuesta final de texto completada
        if (m_streamedContent.trimmed().isEmpty()) {
            emit errorOccurred(QStringLiteral("El modelo devolvió una respuesta vacía."));
        } else {
            const QString cleanForMemory = stripThinkingBlocks(m_streamedContent);
            m_memory.addAssistantMessage(cleanForMemory.isEmpty() ? m_streamedContent : cleanForMemory);
            emit messageReceived(m_streamedContent);
        }

        emit finishedGenerating();
    });
}
