#include "ToolRegistry.h"
#include <QJsonDocument>
#include <QJsonParseError>

void ToolRegistry::registerTool(std::unique_ptr<ITool> tool) {
    if (!tool) return;
    const QString toolName = tool->name();
    m_tools.insert(toolName, std::shared_ptr<ITool>(std::move(tool)));
}

bool ToolRegistry::hasTool(const QString &name) const {
    return m_tools.contains(name);
}

ITool* ToolRegistry::getTool(const QString &name) const {
    auto it = m_tools.find(name);
    return (it != m_tools.end()) ? it.value().get() : nullptr;
}

ToolResult ToolRegistry::executeTool(const QString &name, const QString &argumentsJson) const {
    ITool *tool = getTool(name);
    if (!tool) {
        return ToolResult::error(QStringLiteral("La herramienta '%1' no está registrada.").arg(name));
    }

    QJsonObject argsObj;
    if (!argumentsJson.trimmed().isEmpty()) {
        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(argumentsJson.toUtf8(), &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            return ToolResult::error(
                QStringLiteral("JSON de argumentos inválido para '%1': %2")
                    .arg(name, parseError.errorString()));
        }
        argsObj = doc.object();
    }

    return tool->execute(argsObj);
}

QJsonArray ToolRegistry::buildToolsJson() const {
    QJsonArray arr;
    for (auto it = m_tools.constBegin(); it != m_tools.constEnd(); ++it) {
        arr.append(it.value()->toOpenAiToolJson());
    }
    return arr;
}
