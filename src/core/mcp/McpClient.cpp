#include "McpClient.h"
#include <QJsonDocument>
#include <QElapsedTimer>
#include <QCoreApplication>

McpClient::McpClient(const QString &serverName, QObject *parent)
    : QObject(parent)
    , m_serverName(serverName)
{
}

McpClient::~McpClient() {
    disconnectServer();
}

void McpClient::disconnectServer() {
    m_connected = false;
    if (m_process) {
        if (m_process->state() != QProcess::NotRunning) {
            m_process->terminate();
            if (!m_process->waitForFinished(1500)) {
                m_process->kill();
                m_process->waitForFinished(500);
            }
        }
        delete m_process;
        m_process = nullptr;
    }
}

bool McpClient::connectToServer(const QString &command, const QStringList &args, QString &errorOut) {
    disconnectServer();

    m_process = new QProcess(this);
    m_process->start(command, args);

    if (!m_process->waitForStarted(5000)) {
        errorOut = QStringLiteral("No se pudo iniciar el binario MCP '%1'.").arg(command);
        disconnectServer();
        return false;
    }

    // 1. Handshake MCP: initialize
    QJsonObject clientInfo{
        {"name", "FronterMind-Harness"},
        {"version", "0.3.0"}
    };
    QJsonObject initParams{
        {"protocolVersion", "2024-11-05"},
        {"capabilities", QJsonObject{}},
        {"clientInfo", clientInfo}
    };

    const QJsonObject initResp = sendRpcRequest(QStringLiteral("initialize"), initParams, 10000, errorOut);
    if (initResp.isEmpty() || initResp.contains("error")) {
        if (errorOut.isEmpty()) {
            errorOut = initResp["error"].toObject()["message"].toString(QStringLiteral("Fallo en handshake initialize de MCP."));
        }
        disconnectServer();
        return false;
    }

    // 2. Confirmar inicialización
    sendRpcNotification(QStringLiteral("notifications/initialized"));

    // 3. Descubrir herramientas expuestas por el servidor MCP: tools/list
    const QJsonObject listResp = sendRpcRequest(QStringLiteral("tools/list"), QJsonObject{}, 10000, errorOut);
    if (listResp.contains("result")) {
        const QJsonArray toolsArr = listResp["result"].toObject()["tools"].toArray();
        m_tools.clear();
        for (const QJsonValue &val : toolsArr) {
            const QJsonObject tObj = val.toObject();
            McpToolInfo info;
            info.serverName  = m_serverName;
            info.name        = tObj["name"].toString();
            info.description = tObj["description"].toString();
            info.inputSchema = tObj["inputSchema"].toObject();
            if (info.inputSchema.isEmpty()) {
                info.inputSchema = QJsonObject{{"type", "object"}, {"properties", QJsonObject{}}};
            }
            m_tools.append(info);
        }
    }

    m_connected = true;
    return true;
}

void McpClient::sendRpcNotification(const QString &method, const QJsonObject &params) {
    if (!m_process || m_process->state() != QProcess::Running) return;

    QJsonObject msg{
        {"jsonrpc", "2.0"},
        {"method", method}
    };
    if (!params.isEmpty()) {
        msg["params"] = params;
    }

    QByteArray line = QJsonDocument(msg).toJson(QJsonDocument::Compact) + "\n";
    m_process->write(line);
    m_process->waitForBytesWritten(1000);
}

QJsonObject McpClient::sendRpcRequest(const QString &method, const QJsonObject &params, int timeoutMs, QString &errorOut) {
    if (!m_process || m_process->state() != QProcess::Running) {
        errorOut = QStringLiteral("El proceso MCP no está en ejecución.");
        return {};
    }

    const int reqId = m_nextId++;
    QJsonObject req{
        {"jsonrpc", "2.0"},
        {"id", reqId},
        {"method", method},
        {"params", params}
    };

    const QByteArray payload = QJsonDocument(req).toJson(QJsonDocument::Compact) + "\n";
    m_process->write(payload);
    m_process->waitForBytesWritten(2000);

    QElapsedTimer timer;
    timer.start();
    QByteArray buffer;

    while (timer.elapsed() < timeoutMs) {
        if (m_process->waitForReadyRead(200)) {
            buffer += m_process->readAllStandardOutput();
            while (buffer.contains('\n')) {
                const int nlIdx = buffer.indexOf('\n');
                const QByteArray line = buffer.left(nlIdx).trimmed();
                buffer.remove(0, nlIdx + 1);

                if (line.isEmpty()) continue;
                const QJsonDocument doc = QJsonDocument::fromJson(line);
                if (doc.isObject()) {
                    const QJsonObject obj = doc.object();
                    if (obj.contains("id") && obj["id"].toInt() == reqId) {
                        return obj;
                    }
                }
            }
        }
        if (m_process->state() == QProcess::NotRunning) {
            errorOut = QStringLiteral("El servidor MCP se cerró inesperadamente: ")
                       + QString::fromUtf8(m_process->readAllStandardError());
            return {};
        }
    }

    errorOut = QStringLiteral("Timeout esperando respuesta JSON-RPC del servidor MCP.");
    return {};
}

ToolResult McpClient::callTool(const QString &toolName, const QJsonObject &arguments) {
    QString err;
    QJsonObject params{
        {"name", toolName},
        {"arguments", arguments}
    };

    const QJsonObject resp = sendRpcRequest(QStringLiteral("tools/call"), params, 25000, err);
    if (resp.isEmpty()) {
        return ToolResult::error(err);
    }
    if (resp.contains("error")) {
        return ToolResult::error(resp["error"].toObject()["message"].toString());
    }

    const QJsonObject resultObj = resp["result"].toObject();
    const bool isErr = resultObj["isError"].toBool(false);
    const QJsonArray contentArr = resultObj["content"].toArray();

    QStringList texts;
    for (const QJsonValue &v : contentArr) {
        const QJsonObject item = v.toObject();
        if (item["type"].toString() == QStringLiteral("text")) {
            texts.append(item["text"].toString());
        }
    }

    const QString finalOutput = texts.join('\n');
    return isErr ? ToolResult::error(finalOutput) : ToolResult::success(finalOutput);
}
