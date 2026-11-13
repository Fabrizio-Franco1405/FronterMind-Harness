#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class AgentEngine : public QObject {
    Q_OBJECT
public:
    explicit AgentEngine(QObject *parent = nullptr);

    void sendMessage(const QString &modelName, const QString &text);

signals:
    void messageReceived(const QString &response);
    void errorOccurred(const QString &error);
    void startedGenerating();
    void finishedGenerating();
    void tokensUpdated(int promptTokens, int completionTokens, int totalTokens);

private:
    QNetworkAccessManager *m_networkManager;
};
