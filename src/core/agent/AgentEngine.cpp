#include "AgentEngine.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QDebug>

AgentEngine::AgentEngine(QObject *parent) : QObject(parent) {
    m_networkManager = new QNetworkAccessManager(this);
}

void AgentEngine::sendMessage(const QString &modelName, const QString &text) {
    emit startedGenerating();

    QUrl url("http://127.0.0.1:8080/v1/chat/completions");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject messageObj;
    messageObj["role"] = "user";
    messageObj["content"] = text;

    QJsonArray messagesArray;
    messagesArray.append(messageObj);

    QJsonObject rootObj;
    rootObj["messages"] = messagesArray;
    rootObj["model"] = "local";
    rootObj["temperature"] = 0.7;
    rootObj["stream"] = false;

    QJsonDocument doc(rootObj);
    QNetworkReply *reply = m_networkManager->post(request, doc.toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            // ESTO NOS DIRÁ EN LA TERMINAL QUÉ DEVUELVE EL SERVIDOR
            qDebug() << "RAW RESPONSE FROM LLAMA.CPP:" << responseData;

            QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
            QJsonObject jsonObj = jsonResponse.object();

            // 1. Leer Tokens
            if (jsonObj.contains("usage")) {
                QJsonObject usage = jsonObj["usage"].toObject();
                emit tokensUpdated(usage["prompt_tokens"].toInt(), usage["completion_tokens"].toInt(), usage["total_tokens"].toInt());
            }

            // 2. Leer Contenido
            if (jsonObj.contains("choices")) {
                QJsonArray choices = jsonObj["choices"].toArray();
                if (!choices.isEmpty()) {
                    QJsonObject choice = choices[0].toObject();
                    QJsonObject message = choice["message"].toObject();
                    QString content = message["content"].toString();

                    // Qwen 3 a veces usa reasoning_content
                    if (content.isEmpty() && message.contains("reasoning_content")) {
                        content = message["reasoning_content"].toString();
                    }

                    if (content.isEmpty()) {
                        emit errorOccurred("El modelo devolvió texto vacío.");
                    } else {
                        emit messageReceived(content);
                    }
                } else {
                    emit errorOccurred("El array 'choices' vino vacío.");
                }
            } else {
                emit errorOccurred("La respuesta no tiene el campo 'choices'.");
            }
        } else {
            emit errorOccurred(reply->errorString());
        }
        emit finishedGenerating();
        reply->deleteLater();
    });
}
