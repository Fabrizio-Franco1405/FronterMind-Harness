#pragma once
#include "ITool.h"
#include "ToolRegistry.h"

class WebSearchTool final : public ITool {
public:
    QString name() const override { return QStringLiteral("web_search"); }
    QString description() const override {
        return QStringLiteral("Busca información actualizada en internet y devuelve una lista de títulos, URLs y resúmenes.");
    }
    QJsonObject parametersSchema() const override;
    ToolResult execute(const QJsonObject &arguments) override;
};

class HttpFetchTool final : public ITool {
public:
    QString name() const override { return QStringLiteral("http_fetch"); }
    QString description() const override {
        return QStringLiteral("Descarga el contenido de una página web o endpoint HTTP/JSON y lo devuelve como texto limpio.");
    }
    QJsonObject parametersSchema() const override;
    ToolResult execute(const QJsonObject &arguments) override;
};

void registerWebTools(ToolRegistry &registry);
