#include "AppController.h"
#include <QDebug>

AppController::AppController(QObject *parent) : QObject(parent) {}

QString AppController::statusText() const {
    return m_statusText;
}

void AppController::setStatusText(const QString &text) {
    if (m_statusText == text) return;
    m_statusText = text;
    emit statusTextChanged();
}

void AppController::handleUserAction(const QString &message) {
    qDebug() << "UI envió mensaje al Core:" << message;

    // Aquí en el futuro llamaremos a FrontCore (AgentEngine)
    // Por ahora, simulamos una respuesta del motor:
    setStatusText("Recibido: " + message);
}
