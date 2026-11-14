#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include "AppController.h"
#include "IconImageProvider.h"
#include "ThemeManager.h"
#include <oclero/qlementine/icons/QlementineIcons.hpp>

int main(int argc, char *argv[])
{

    oclero::qlementine::icons::initializeIconTheme();
    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/assets/logo.png"));

    // IMPORTANTE: Instanciar los controladores ANTES del QQmlApplicationEngine
    // para que vivan más tiempo que la UI al cerrar la aplicación.
    AppController appController;
    ThemeManager themeManager;

    QQmlApplicationEngine engine;
    engine.addImageProvider("icon", new IconImageProvider());

    engine.rootContext()->setContextProperty("backend", &appController);
    engine.rootContext()->setContextProperty("theme", &themeManager);

    engine.addImportPath(":/qt/qml/");
    engine.loadFromModule("FronterMind.UI", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
