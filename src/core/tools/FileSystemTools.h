#pragma once
#include "ITool.h"
#include "ToolRegistry.h"

// 1. Listar archivos y carpetas
class ListDirectoryTool final : public ITool {
public:
    QString name() const override { return QStringLiteral("list_directory"); }
    QString description() const override {
        return QStringLiteral("Lista los archivos y subdirectorios de una ruta dada.");
    }
    QJsonObject parametersSchema() const override;
    ToolResult execute(const QJsonObject &arguments) override;
};

// 2. Leer contenido de un archivo
class ReadFileTool final : public ITool {
public:
    QString name() const override { return QStringLiteral("read_file"); }
    QString description() const override {
        return QStringLiteral("Lee el contenido de un archivo de texto o código fuente. Soporta lectura parcial por líneas.");
    }
    QJsonObject parametersSchema() const override;
    ToolResult execute(const QJsonObject &arguments) override;
};

// 3. Crear o sobrescribir un archivo
class WriteFileTool final : public ITool {
public:
    QString name() const override { return QStringLiteral("write_file"); }
    QString description() const override {
        return QStringLiteral("Crea un archivo nuevo o sobrescribe uno existente con el contenido proporcionado.");
    }
    QJsonObject parametersSchema() const override;
    ToolResult execute(const QJsonObject &arguments) override;
};

// 4. Editar un bloque específico dentro de un archivo
class EditFileTool final : public ITool {
public:
    QString name() const override { return QStringLiteral("edit_file"); }
    QString description() const override {
        return QStringLiteral("Reemplaza un bloque exacto de texto (search_text) por uno nuevo (replace_text) en un archivo.");
    }
    QJsonObject parametersSchema() const override;
    ToolResult execute(const QJsonObject &arguments) override;
};

// Helper para registrar todas las herramientas de archivos en una sola llamada
void registerFileSystemTools(ToolRegistry &registry);
