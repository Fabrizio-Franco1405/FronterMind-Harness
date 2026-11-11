#include "EngineManager.h"
#include <QCoreApplication>
#include <QDebug>

#ifdef Q_OS_LINUX
#include <sys/prctl.h>
#include <signal.h>
#include <unistd.h>
#endif

EngineManager::EngineManager(QObject *parent) : QObject(parent) {
    m_killTimer = new QTimer(this);
    m_killTimer->setSingleShot(true);
    m_killTimer->setInterval(5000); // 5 segundos de gracia antes de SIGKILL

    // Si el timer expira, forzamos la muerte del proceso (SIGKILL)
    connect(m_killTimer, &QTimer::timeout, this, [this]() {
        if (m_process && m_process->state() != QProcess::NotRunning) {
            appendLog("[Sistema] El servidor no respondió a SIGTERM. Enviando SIGKILL...");
            m_process->kill();
        }
    });

    // Aseguramos la parada del motor cuando la aplicación se cierre
    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, &EngineManager::stopServer);
}

EngineManager::~EngineManager() {
    stopServer();
}

void EngineManager::startServer(const QString &binaryPath, const QString &modelPath, int gpuLayers, int contextSize) {
    if (isRunning()) {
        appendLog("[Sistema] El motor ya está en ejecución.");
        return;
    }

    m_process = new QProcess(this);

    connect(m_process, &QProcess::readyReadStandardOutput, this, &EngineManager::onReadyReadStandardOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &EngineManager::onReadyReadStandardError);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &EngineManager::onProcessFinished);

    #ifdef Q_OS_LINUX
    m_process->setChildProcessModifier([]() {
        ::prctl(PR_SET_PDEATHSIG, SIGTERM);
    });
    #endif

    QStringList args;
    args << "-m" << modelPath
         << "-ngl" << QString::number(gpuLayers)
         << "-c" << QString::number(contextSize) // <-- NUEVO: Pasar contexto
         << "--host" << "127.0.0.1"
         << "--port" << "8080";

    appendLog("[Sistema] Iniciando: " + binaryPath + " " + args.join(" "));

    m_process->start(binaryPath, args);

    if (!m_process->waitForStarted(5000)) {
        appendLog("[Error] No se pudo iniciar el proceso. Verifica la ruta del binario.");
        emit serverError("No se pudo iniciar llama-server.");
        delete m_process;
        m_process = nullptr;
        return;
    }

    emit runningChanged();
}

void EngineManager::stopServer() {
    if (!m_process || m_process->state() == QProcess::NotRunning) {
        return;
    }

    appendLog("[Sistema] Deteniendo servidor (SIGTERM)...");
    m_process->terminate(); // Envía SIGTERM
    m_killTimer->start();   // Inicia el temporizador de seguridad (SIGKILL)
}

void EngineManager::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    m_killTimer->stop();

    if (exitStatus == QProcess::CrashExit) {
        appendLog("[Error] El servidor se cerró inesperadamente (Crash).");
    } else {
        appendLog("[Sistema] Servidor detenido correctamente (Código: " + QString::number(exitCode) + ").");
    }

    if (m_process) {
        m_process->deleteLater();
        m_process = nullptr;
    }
    emit runningChanged();
}

void EngineManager::onReadyReadStandardOutput() {
    if (m_process) {
        appendLog(QString::fromUtf8(m_process->readAllStandardOutput()));
    }
}

void EngineManager::onReadyReadStandardError() {
    if (m_process) {
        appendLog(QString::fromUtf8(m_process->readAllStandardError()));
    }
}

void EngineManager::appendLog(const QString &text) {
    m_serverLog += text;
    // Limitar el log para no consumir memoria innecesaria
    if (m_serverLog.length() > 50000) {
        m_serverLog = m_serverLog.right(40000);
    }
    emit serverLogChanged();
}
