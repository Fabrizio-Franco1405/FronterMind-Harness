#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QIcon>
#include "AppController.h"
#include "IconImageProvider.h"
#include "ThemeManager.h"
#include <oclero/qlementine/icons/QlementineIcons.hpp>

#ifdef _WIN32
#include <windows.h>
#include <dwmapi.h>
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#endif

int main(int argc, char *argv[])
{
    // Forzar el motor de estilo unificado "Basic" antes de crear QGuiApplication
    // Esto garantiza que Windows y Linux se vean 100% idénticos (sin menús ni botones blancos Win32)
    QQuickStyle::setStyle("Basic");

    oclero::qlementine::icons::initializeIconTheme();
    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/assets/logo.png"));

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

#ifdef _WIN32
    // Pintar la barra de título nativa de Windows 10/11 en Modo Oscuro
    if (auto *win = qobject_cast<QQuickWindow *>(engine.rootObjects().first())) {
        HWND hwnd = reinterpret_cast<HWND>(win->winId());
        BOOL useDarkMode = TRUE;
        DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkMode, sizeof(useDarkMode));
    }
#endif

    return app.exec();
}
