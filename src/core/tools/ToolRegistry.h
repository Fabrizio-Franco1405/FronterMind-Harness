#pragma once
#include "ITool.h"
#include <QHash>
#include <QJsonArray>
#include <QString>
#include <memory>

class ToolRegistry {
public:
    ToolRegistry() = default;

    void registerTool(std::unique_ptr<ITool> tool);
    bool hasTool(const QString &name) const;
    ITool* getTool(const QString &name) const;

    // Ejecuta una herramienta a partir del nombre y el string JSON de argumentos que envía el LLM
    ToolResult executeTool(const QString &name, const QString &argumentsJson) const;

    // Construye el array "tools" para el payload de llama-server
    QJsonArray buildToolsJson() const;

    bool isEmpty() const { return m_tools.isEmpty(); }

private:
    QHash<QString, std::shared_ptr<ITool>> m_tools;
};
