#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "AppController.h"
#include "IconImageProvider.h"
#include <oclero/qlementine/icons/QlementineIcons.hpp>

int main(int argc, char *argv[])
{
    oclero::qlementine::icons::initializeIconTheme();
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    engine.addImageProvider("icon", new IconImageProvider());

    AppController appController;
    engine.rootContext()->setContextProperty("backend", &appController);

    engine.addImportPath(":/qt/qml/");
    engine.loadFromModule("FronterMind.UI", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
