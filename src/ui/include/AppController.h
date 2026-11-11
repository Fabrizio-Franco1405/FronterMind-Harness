#pragma once
#include <QObject>
#include <QString>

class AppController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString statusText READ statusText WRITE setStatusText NOTIFY statusTextChanged)

public:
    explicit AppController(QObject *parent = nullptr);

    QString statusText() const;
    void setStatusText(const QString &text);

    // Esta función será llamada desde QML
    Q_INVOKABLE void handleUserAction(const QString &message);

signals:
    void statusTextChanged();

private:
    QString m_statusText = "Esperando acción...";
};
