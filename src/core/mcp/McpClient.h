#pragma once
#include <QObject>
#include <QProcess>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QStringList>
#include "tools/ITool.h"
#include "tools/ToolRegistry.h"

struct McpToolInfo {
    QString serverName;
    QString name;
    QString description;
    QJsonObject inputSchema;
};

class McpClient : public QObject {
    Q_OBJECT
public:
    explicit McpClient(const QString &serverName, QObject *parent = nullptr);
    ~McpClient() override;

    QString serverName() const { return m_serverName; }
    bool isConnected() const { return m_connected; }

    // Inicia el proceso MCP (ej. command="npx", args={"-y", "@modelcontextprotocol/server-everything"})
    // realiza el handshake JSON-RPC "initialize" y descarga "tools/list"
    bool connectToServer(const QString &command, const QStringList &args, QString &errorOut);
    void disconnectServer();

    // Invoca "tools/call" sobre el servidor MCP vía JSON-RPC 2.0
    ToolResult callTool(const QString &toolName, const QJsonObject &arguments);

    const QList<McpToolInfo>& discoveredTools() const { return m_tools; }

private:
    QJsonObject sendRpcRequest(const QString &method, const QJsonObject &params, int timeoutMs, QString &errorOut);
    void sendRpcNotification(const QString &method, const QJsonObject &params = {});

    QString m_serverName;
    QProcess *m_process = nullptr;
    bool m_connected = false;
    int m_nextId = 1;
    QList<McpToolInfo> m_tools;
};

// Adaptador que envuelve una herramienta de un servidor MCP como un ITool nativo para ToolRegistry
class McpBridgeTool final : public ITool {
public:
    McpBridgeTool(McpClient *client, const McpToolInfo &info)
        : m_client(client), m_info(info) {}

    QString name() const override { return m_info.name; }
    QString description() const override {
        return QStringLiteral("[MCP:%1] %2").arg(m_info.serverName, m_info.description);
    }
    QJsonObject parametersSchema() const override { return m_info.inputSchema; }
    ToolResult execute(const QJsonObject &arguments) override {
        if (!m_client || !m_client->isConnected()) {
            return ToolResult::error(QStringLiteral("El servidor MCP '%1' no está conectado.").arg(m_info.serverName));
        }
        return m_client->callTool(m_info.name, arguments);
    }

private:
    McpClient *m_client = nullptr;
    McpToolInfo m_info;
};
