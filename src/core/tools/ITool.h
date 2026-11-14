#pragma once
#include <QString>
#include <QJsonObject>

struct ToolResult {
    bool isError = false;
    QString output;

    static ToolResult success(const QString &text) {
        return {false, text};
    }

    static ToolResult error(const QString &errText) {
        return {true, QStringLiteral("Error: ") + errText};
    }
};

class ITool {
public:
    virtual ~ITool() = default;

    // Identificador único que usa el LLM (ej. "read_file")
    virtual QString name() const = 0;

    // Descripción clara para que el LLM sepa cuándo invocarla
    virtual QString description() const = 0;

    // JSON Schema de los parámetros esperados
    virtual QJsonObject parametersSchema() const = 0;

    // Ejecución síncrona/rápida de la herramienta
    virtual ToolResult execute(const QJsonObject &arguments) = 0;

    // Genera el objeto JSON compatible con OpenAI / llama-server tool calling
    QJsonObject toOpenAiToolJson() const {
        QJsonObject fnObj;
        fnObj["name"] = name();
        fnObj["description"] = description();
        fnObj["parameters"] = parametersSchema();

        QJsonObject toolObj;
        toolObj["type"] = QStringLiteral("function");
        toolObj["function"] = fnObj;
        return toolObj;
    }
};
