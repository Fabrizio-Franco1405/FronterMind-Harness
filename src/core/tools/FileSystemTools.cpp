#include "FileSystemTools.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>

// --- 1. LIST DIRECTORY ---
QJsonObject ListDirectoryTool::parametersSchema() const {
    QJsonObject pathProp{
        {"type", "string"},
        {"description", "Ruta del directorio a inspeccionar (ej. '.' para el directorio actual o una ruta absoluta)."}
    };
    QJsonObject properties{{"path", pathProp}};
    return QJsonObject{
        {"type", "object"},
        {"properties", properties},
        {"required", QJsonArray{"path"}}
    };
}

ToolResult ListDirectoryTool::execute(const QJsonObject &arguments) {
    const QString path = arguments.value("path").toString(QStringLiteral(".")).trimmed();
    QDir dir(path);
    if (!dir.exists()) {
        return ToolResult::error(QStringLiteral("El directorio no existe: %1").arg(path));
    }

    const QFileInfoList entries = dir.entryInfoList(
        QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot,
        QDir::DirsFirst | QDir::Name
    );

    QStringList lines;
    lines.append(QStringLiteral("Directorio: %1 (%2 elementos)").arg(dir.absolutePath()).arg(entries.size()));
    for (const QFileInfo &info : entries) {
        if (info.isDir()) {
            lines.append(QStringLiteral("[DIR]  %1/").arg(info.fileName()));
        } else {
            lines.append(QStringLiteral("[FILE] %1 (%2 bytes)").arg(info.fileName()).arg(info.size()));
        }
    }
    return ToolResult::success(lines.join('\n'));
}

// --- 2. READ FILE ---
QJsonObject ReadFileTool::parametersSchema() const {
    QJsonObject properties{
        {"path", QJsonObject{{"type", "string"}, {"description", "Ruta del archivo a leer."}}},
        {"start_line", QJsonObject{{"type", "integer"}, {"description", "Línea inicial (1-indexed, opcional)."}}},
        {"end_line", QJsonObject{{"type", "integer"}, {"description", "Línea final (1-indexed, opcional)."}}}
    };
    return QJsonObject{
        {"type", "object"},
        {"properties", properties},
        {"required", QJsonArray{"path"}}
    };
}

ToolResult ReadFileTool::execute(const QJsonObject &arguments) {
    const QString path = arguments.value("path").toString().trimmed();
    if (path.isEmpty()) {
        return ToolResult::error(QStringLiteral("Parámetro 'path' vacío."));
    }

    QFile file(path);
    if (!file.exists()) {
        return ToolResult::error(QStringLiteral("El archivo no existe: %1").arg(path));
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return ToolResult::error(QStringLiteral("No se pudo abrir el archivo para lectura: %1").arg(path));
    }

    const QString content = QString::fromUtf8(file.readAll());
    const QStringList allLines = content.split('\n');

    int startLine = arguments.value("start_line").toInt(1);
    int endLine   = arguments.value("end_line").toInt(allLines.size());

    startLine = qBound(1, startLine, qMax(1, static_cast<int>(allLines.size())));
    endLine   = qBound(startLine, endLine, static_cast<int>(allLines.size()));

    // Límite de seguridad para no desbordar el contexto si leen un archivo gigante
    constexpr int maxLinesPerRead = 300;
    bool truncated = false;
    if (endLine - startLine + 1 > maxLinesPerRead) {
        endLine = startLine + maxLinesPerRead - 1;
        truncated = true;
    }

    QStringList numberedLines;
    numberedLines.reserve(endLine - startLine + 2);
    for (int i = startLine; i <= endLine; ++i) {
        numberedLines.append(QStringLiteral("%1 | %2").arg(i, 4).arg(allLines[i - 1]));
    }

    if (truncated) {
        numberedLines.append(QStringLiteral("\n[... Archivo truncado a %1 líneas. Usa start_line y end_line para leer el resto ...]").arg(maxLinesPerRead));
    }

    return ToolResult::success(numberedLines.join('\n'));
}

// --- 3. WRITE FILE ---
QJsonObject WriteFileTool::parametersSchema() const {
    QJsonObject properties{
        {"path", QJsonObject{{"type", "string"}, {"description", "Ruta del archivo a crear o sobrescribir."}}},
        {"content", QJsonObject{{"type", "string"}, {"description", "Contenido completo a escribir en el archivo."}}}
    };
    return QJsonObject{
        {"type", "object"},
        {"properties", properties},
        {"required", QJsonArray{"path", "content"}}
    };
}

ToolResult WriteFileTool::execute(const QJsonObject &arguments) {
    const QString path = arguments.value("path").toString().trimmed();
    const QString content = arguments.value("content").toString();

    if (path.isEmpty()) {
        return ToolResult::error(QStringLiteral("Parámetro 'path' vacío."));
    }

    QFileInfo info(path);
    QDir parentDir = info.dir();
    if (!parentDir.exists() && !parentDir.mkpath(QStringLiteral("."))) {
        return ToolResult::error(QStringLiteral("No se pudo crear el directorio padre: %1").arg(parentDir.absolutePath()));
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return ToolResult::error(QStringLiteral("No se pudo abrir el archivo para escritura: %1").arg(path));
    }

    const QByteArray bytes = content.toUtf8();
    file.write(bytes);
    file.close();

    return ToolResult::success(
        QStringLiteral("Archivo escrito correctamente en '%1' (%2 bytes).").arg(info.absoluteFilePath()).arg(bytes.size())
    );
}

// --- 4. EDIT FILE ---
QJsonObject EditFileTool::parametersSchema() const {
    QJsonObject properties{
        {"path", QJsonObject{{"type", "string"}, {"description", "Ruta del archivo a modificar."}}},
        {"search_text", QJsonObject{{"type", "string"}, {"description", "Fragmento exacto existente que se desea reemplazar."}}},
        {"replace_text", QJsonObject{{"type", "string"}, {"description", "Nuevo texto que reemplazará a search_text."}}}
    };
    return QJsonObject{
        {"type", "object"},
        {"properties", properties},
        {"required", QJsonArray{"path", "search_text", "replace_text"}}
    };
}

ToolResult EditFileTool::execute(const QJsonObject &arguments) {
    const QString path = arguments.value("path").toString().trimmed();
    const QString searchText = arguments.value("search_text").toString();
    const QString replaceText = arguments.value("replace_text").toString();

    if (path.isEmpty() || searchText.isEmpty()) {
        return ToolResult::error(QStringLiteral("Parámetros 'path' y 'search_text' son obligatorios."));
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return ToolResult::error(QStringLiteral("No se pudo leer el archivo: %1").arg(path));
    }

    QString content = QString::fromUtf8(file.readAll());
    file.close();

    const int occurrences = content.count(searchText);
    if (occurrences == 0) {
        return ToolResult::error(QStringLiteral("No se encontró el bloque 'search_text' exactamente igual dentro de '%1'.").arg(path));
    }

    const int firstIdx = content.indexOf(searchText);
    content.replace(firstIdx, searchText.length(), replaceText);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return ToolResult::error(QStringLiteral("No se pudo guardar el archivo editado: %1").arg(path));
    }

    file.write(content.toUtf8());
    file.close();

    return ToolResult::success(QStringLiteral("Archivo '%1' editado con éxito.").arg(path));
}

void registerFileSystemTools(ToolRegistry &registry) {
    registry.registerTool(std::make_unique<ListDirectoryTool>());
    registry.registerTool(std::make_unique<ReadFileTool>());
    registry.registerTool(std::make_unique<WriteFileTool>());
    registry.registerTool(std::make_unique<EditFileTool>());
}
