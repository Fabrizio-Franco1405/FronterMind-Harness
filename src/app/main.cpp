// ============================================================================
// SECCIÓN 1: CABECERAS DEL SISTEMA Y MÓDULOS DE QT6
// ============================================================================
#include <QApplication>          // Requerido para usar el Explorador de Archivos nativo (KDE Dolphin / Win32)
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QIcon>
#include <QWindow>
#include <oclero/qlementine/icons/QlementineIcons.hpp>

// ============================================================================
// SECCIÓN 2: CONTROLADORES DE INTERFAZ Y PROVEEDORES DE ICONOS
// ============================================================================
#include "controllers/AppController.h"
#include "controllers/ThemeManager.h"
#include "controllers/IconImageProvider.h"

// ============================================================================
// SECCIÓN 3: CABECERAS NATIVAS DE WINDOWS (DWM DARK TITLEBAR)
// ============================================================================
#ifdef _WIN32
#include <windows.h>
#include <dwmapi.h>
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#endif

// ============================================================================
// SECCIÓN 4: PUNTO DE ENTRADA PRINCIPAL DE LA APLICACIÓN (MAIN)
// ============================================================================
int main(int argc, char *argv[])
{
    // --- 4.1: Configuración del Estilo Visual Unificado ---
    // Fuerza el estilo "Basic" de QtQuickControls2 para que todos los colores QML
    // respondan al ThemeManager tanto en Linux como en Windows.
    QQuickStyle::setStyle("Basic");

    // --- 4.2: Inicialización de Iconografía Vectorial (Qlementine) ---
    oclero::qlementine::icons::initializeIconTheme();

    // --- 4.3: Instancia de Aplicación con Soporte de Diálogos Nativos ---
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/assets/logo.png"));

    // --- 4.4: Instanciación de Controladores Backend y Temas ---
    AppController appController;
    ThemeManager themeManager;

    // --- 4.5: Configuración del Motor QML e Inyección de Contexto ---
    QQmlApplicationEngine engine;
    engine.addImageProvider("icon", new IconImageProvider());

    engine.rootContext()->setContextProperty("backend", &appController);
    engine.rootContext()->setContextProperty("theme", &themeManager);

    // --- 4.6: Carga del Módulo Principal de Interfaz (FronterMind.UI) ---
    engine.addImportPath(":/qt/qml/");
    engine.loadFromModule("FronterMind.UI", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    // --- 4.7: Activación de Barra de Título Oscura Nativa en Windows 10/11 ---
#ifdef _WIN32
    if (auto *window = qobject_cast<QWindow *>(engine.rootObjects().first())) {
        HWND hwnd = reinterpret_cast<HWND>(window->winId());
        BOOL useDarkMode = TRUE;
        DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkMode, sizeof(useDarkMode));
    }
#endif

    // --- 4.8: Bucle de Eventos Principal ---
    return app.exec();
}
