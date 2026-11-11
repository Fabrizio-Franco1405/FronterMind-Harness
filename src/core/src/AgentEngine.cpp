#include "AgentEngine.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>

AgentEngine::AgentEngine(QObject *parent) : QObject(parent) {
    m_networkManager = new QNetworkAccessManager(this);
}

void AgentEngine::sendMessage(const QString &modelName, const QString &text) {
    emit startedGenerating();

    QUrl url("http://127.0.0.1:8080/v1/chat/completions");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Construimos el JSON estilo OpenAI
    QJsonObject messageObj;
    messageObj["role"] = "user";
    messageObj["content"] = text;

    QJsonArray messagesArray;
    messagesArray.append(messageObj);

    QJsonObject rootObj;
    rootObj["messages"] = messagesArray;
    rootObj["model"] = "local";
    rootObj["temperature"] = 0.7;

    QJsonDocument doc(rootObj);

    QNetworkReply *reply = m_networkManager->post(request, doc.toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
            QJsonObject jsonObj = jsonResponse.object();

            if (jsonObj.contains("choices")) {
                QJsonArray choices = jsonObj["choices"].toArray();
                if (!choices.isEmpty()) {
                    QJsonObject choice = choices[0].toObject();
                    QJsonObject message = choice["message"].toObject();
                    QString content = message["content"].toString();
                    emit messageReceived(content);
                }
            } else {
                emit errorOccurred("Formato de respuesta no reconocido.");
            }
        } else {
            emit errorOccurred(reply->errorString());
        }

        emit finishedGenerating();
        reply->deleteLater();
    });
}
