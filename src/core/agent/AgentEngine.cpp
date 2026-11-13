#include "AgentEngine.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QUrl>
#include <QDebug>

AgentEngine::AgentEngine(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_memory(4096)
{
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

    // 1. Guardar el mensaje del usuario en la memoria de conversación (aplica ventana deslizante automática)
    m_memory.addUserMessage(text);

    // Emitir estimación inmediata de tokens antes de recibir respuesta del servidor
    emit tokensUpdated(m_memory.usedTokens(), 0, m_memory.usedTokens());

    // 2. Construir petición HTTP con todo el historial de mensajes
    QUrl url(m_serverUrl);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    QJsonObject rootObj;
    rootObj["model"] = QStringLiteral("local");
    rootObj["messages"] = m_memory.buildApiMessagesPayload();
    rootObj["temperature"] = 0.7;
    rootObj["stream"] = false;

    QJsonDocument doc(rootObj);
    QNetworkReply *reply = m_networkManager->post(request, doc.toJson(QJsonDocument::Compact));

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            const QByteArray responseData = reply->readAll();
            const QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
            const QJsonObject jsonObj = jsonResponse.object();

            // 1. Actualizar conteo real de tokens desde llama-server
            if (jsonObj.contains("usage")) {
                const QJsonObject usage = jsonObj["usage"].toObject();
                const int promptTok = usage["prompt_tokens"].toInt();
                const int compTok   = usage["completion_tokens"].toInt();
                const int totalTok  = usage["total_tokens"].toInt();

                m_memory.updateActualTokenUsage(promptTok, compTok);
                emit tokensUpdated(promptTok, compTok, totalTok);
            }

            // 2. Extraer respuesta del modelo y guardarla en ConversationMemory
            if (jsonObj.contains("choices")) {
                const QJsonArray choices = jsonObj["choices"].toArray();
                if (!choices.isEmpty()) {
                    const QJsonObject choice = choices[0].toObject();
                    const QJsonObject message = choice["message"].toObject();
                    QString content = message["content"].toString();

                    if (content.isEmpty() && message.contains("reasoning_content")) {
                        content = message["reasoning_content"].toString();
                    }

                    if (content.isEmpty()) {
                        emit errorOccurred(QStringLiteral("El modelo devolvió una respuesta vacía."));
                    } else {
                        // Guardamos la respuesta en la memoria sin bloques <think> para no desperdiciar contexto
                        const QString cleanForMemory = stripThinkingBlocks(content);
                        m_memory.addAssistantMessage(cleanForMemory.isEmpty() ? content : cleanForMemory);

                        emit messageReceived(content);
                    }
                } else {
                    emit errorOccurred(QStringLiteral("El servidor devolvió 'choices' vacío."));
                }
            } else {
                emit errorOccurred(QStringLiteral("Respuesta inválida del servidor (sin campo 'choices')."));
            }
        } else {
            emit errorOccurred(reply->errorString());
        }

        emit finishedGenerating();
        reply->deleteLater();
    });
}
