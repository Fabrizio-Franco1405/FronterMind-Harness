#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext> // <-- Nuevo include
#include "AppController.h" // <-- Nuevo include

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    // 1. Creamos el controlador
    AppController appController;

    // 2. Lo exponemos a QML como "backend"
    engine.rootContext()->setContextProperty("backend", &appController);

    engine.addImportPath(":/qt/qml/");
    engine.loadFromModule("FronterMind.UI", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
