#pragma once
#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QString>

class EngineManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(QString serverLog READ serverLog NOTIFY serverLogChanged)

public:
    explicit EngineManager(QObject *parent = nullptr);
    ~EngineManager();

    bool isRunning() const { return m_process && m_process->state() == QProcess::Running; }
    QString serverLog() const { return m_serverLog; }

    Q_INVOKABLE void startServer(const QString &binaryPath, const QString &modelPath, int gpuLayers = 99);
    Q_INVOKABLE void stopServer();

signals:
    void runningChanged();
    void serverLogChanged();
    void serverError(const QString &error);

private slots:
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QProcess *m_process = nullptr;
    QTimer *m_killTimer = nullptr;
    QString m_serverLog;
    void appendLog(const QString &text);
};
