#include "TerminalTool.h"
#include <QProcess>
#include <QJsonArray>
#include <QDir>

QJsonObject RunShellCommandTool::parametersSchema() const {
    QJsonObject properties{
        {"command", QJsonObject{
            {"type", "string"},
            {"description", "El comando completo a ejecutar (ej. 'cmake --build build' o 'git status')."}
        }},
        {"working_directory", QJsonObject{
            {"type", "string"},
            {"description", "Directorio de trabajo opcional donde se ejecutará el comando."}
        }}
    };

    return QJsonObject{
        {"type", "object"},
        {"properties", properties},
        {"required", QJsonArray{"command"}}
    };
}

ToolResult RunShellCommandTool::execute(const QJsonObject &arguments) {
    const QString command = arguments.value("command").toString().trimmed();
    const QString workDir = arguments.value("working_directory").toString().trimmed();

    if (command.isEmpty()) {
        return ToolResult::error(QStringLiteral("El parámetro 'command' está vacío."));
    }

    QProcess process;
    if (!workDir.isEmpty()) {
        QDir dir(workDir);
        if (!dir.exists()) {
            return ToolResult::error(QStringLiteral("El directorio de trabajo no existe: %1").arg(workDir));
        }
        process.setWorkingDirectory(dir.absolutePath());
    }

#ifdef Q_OS_WIN
    process.start(QStringLiteral("cmd.exe"), QStringList{QStringLiteral("/c"), command});
#else
    process.start(QStringLiteral("/bin/sh"), QStringList{QStringLiteral("-c"), command});
#endif

    if (!process.waitForStarted(3000)) {
        return ToolResult::error(QStringLiteral("No se pudo iniciar el subproceso de terminal."));
    }

    // Timeout de 30 segundos para evitar bloqueos por comandos interactivos o infinitos
    if (!process.waitForFinished(30000)) {
        process.kill();
        process.waitForFinished(1000);
        return ToolResult::error(QStringLiteral("Tiempo de espera agotado (30s). El proceso fue terminado."));
    }

    const QString stdoutStr = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
    const QString stderrStr = QString::fromUtf8(process.readAllStandardError()).trimmed();
    const int exitCode = process.exitCode();

    QString combined;
    combined += QStringLiteral("Exit Code: %1\n").arg(exitCode);
    if (!stdoutStr.isEmpty()) {
        combined += QStringLiteral("--- STDOUT ---\n%1\n").arg(stdoutStr);
    }
    if (!stderrStr.isEmpty()) {
        combined += QStringLiteral("--- STDERR ---\n%1\n").arg(stderrStr);
    }
    if (stdoutStr.isEmpty() && stderrStr.isEmpty()) {
        combined += QStringLiteral("(Comando ejecutado sin salida en consola)");
    }

    // Truncar si la salida de consola es masiva para cuidar los tokens del contexto
    constexpr int maxOutputChars = 8000;
    if (combined.length() > maxOutputChars) {
        combined = combined.left(maxOutputChars) + QStringLiteral("\n[... Salida truncada por longitud ...]");
    }

    return (exitCode == 0) ? ToolResult::success(combined) : ToolResult{true, combined};
}

void registerTerminalTools(ToolRegistry &registry) {
    registry.registerTool(std::make_unique<RunShellCommandTool>());
}
