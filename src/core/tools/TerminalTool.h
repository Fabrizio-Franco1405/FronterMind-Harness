#pragma once
#include "ITool.h"
#include "ToolRegistry.h"

class RunShellCommandTool final : public ITool {
public:
    QString name() const override { return QStringLiteral("run_shell_command"); }

    QString description() const override {
        return QStringLiteral(
            "Ejecuta un comando de terminal en el sistema operativo (sh/bash en Linux/macOS, cmd en Windows) "
            "y devuelve la salida estándar (stdout), errores (stderr) y el código de salida."
        );
    }

    QJsonObject parametersSchema() const override;
    ToolResult execute(const QJsonObject &arguments) override;
};

void registerTerminalTools(ToolRegistry &registry);
