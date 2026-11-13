#include "WebTools.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QRegularExpression>
#include <QJsonArray>

namespace {
QByteArray performSyncGet(const QUrl &url, int timeoutMs, QString &errorOut) {
    QNetworkAccessManager manager;
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader,
                  QStringLiteral("Mozilla/5.0 (X11; Linux x86_64) FronterMind/0.3"));
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply *reply = manager.get(req);
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, [&]() {
        reply->abort();
        loop.quit();
    });

    timer.start(timeoutMs);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        errorOut = reply->errorString();
        reply->deleteLater();
        return {};
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();
    return data;
}

QString htmlToCleanText(QString html) {
    static const QRegularExpression scriptStyleRe(
        R"(<(script|style|noscript|svg|head)[^>]*>[\s\S]*?</\1>)",
        QRegularExpression::CaseInsensitiveOption
    );
    html.remove(scriptStyleRe);

    static const QRegularExpression blockTagRe(R"(</?(p|div|br|li|h[1-6]|tr)[^>]*>)", QRegularExpression::CaseInsensitiveOption);
    html.replace(blockTagRe, QStringLiteral("\n"));

    static const QRegularExpression anyTagRe(R"(<[^>]+>)");
    html.remove(anyTagRe);

    html.replace(QStringLiteral("&nbsp;"), QStringLiteral(" "));
    html.replace(QStringLiteral("&amp;"), QStringLiteral("&"));
    html.replace(QStringLiteral("&lt;"), QStringLiteral("<"));
    html.replace(QStringLiteral("&gt;"), QStringLiteral(">"));
    html.replace(QStringLiteral("&quot;"), QStringLiteral("\""));

    QStringList cleanLines;
    for (const QString &line : html.split('\n')) {
        const QString trimmed = line.simplified();
        if (!trimmed.isEmpty()) {
            cleanLines.append(trimmed);
        }
    }
    return cleanLines.join('\n');
}
} // namespace

// --- 1. WEB SEARCH ---
QJsonObject WebSearchTool::parametersSchema() const {
    QJsonObject properties{
        {"query", QJsonObject{
            {"type", "string"},
            {"description", "Consulta de búsqueda en internet."}
        }}
    };
    return QJsonObject{
        {"type", "object"},
        {"properties", properties},
        {"required", QJsonArray{"query"}}
    };
}

ToolResult WebSearchTool::execute(const QJsonObject &arguments) {
    const QString query = arguments.value("query").toString().trimmed();
    if (query.isEmpty()) {
        return ToolResult::error(QStringLiteral("Parámetro 'query' vacío."));
    }

    QUrl url(QStringLiteral("https://html.duckduckgo.com/html/"));
    QUrlQuery urlQuery;
    urlQuery.addQueryItem(QStringLiteral("q"), query);
    url.setQuery(urlQuery);

    QString netErr;
    const QByteArray rawHtml = performSyncGet(url, 12000, netErr);
    if (rawHtml.isEmpty()) {
        return ToolResult::error(QStringLiteral("Error de red al buscar: %1").arg(netErr));
    }

    const QString htmlStr = QString::fromUtf8(rawHtml);
    static const QRegularExpression resultRe(
        R"re(<a[^>]*class="result__a"[^>]*href="([^"]+)"[^>]*>([\s\S]*?)</a>[\s\S]*?<a[^>]*class="result__snippet"[^>]*>([\s\S]*?)</a>)re",
        QRegularExpression::CaseInsensitiveOption
    );

    QStringList results;
    auto it = resultRe.globalMatch(htmlStr);
    int count = 0;

    while (it.hasNext() && count < 6) {
        auto match = it.next();
        QString link = match.captured(1);
        // Decodificar redirección uddg de DuckDuckGo si está presente
        if (link.contains(QStringLiteral("uddg="))) {
            const QUrl redirectUrl(link.startsWith(QStringLiteral("//")) ? QStringLiteral("https:") + link : link);
            const QString uddg = QUrlQuery(redirectUrl).queryItemValue(QStringLiteral("uddg"), QUrl::FullyDecoded);
            if (!uddg.isEmpty()) link = uddg;
        }

        const QString title = htmlToCleanText(match.captured(2));
        const QString snippet = htmlToCleanText(match.captured(3));

        ++count;
        results.append(QStringLiteral("%1. %2\n   URL: %3\n   Resumen: %4\n")
                           .arg(count)
                           .arg(title, link, snippet));
    }

    if (results.isEmpty()) {
        return ToolResult::success(QStringLiteral("No se encontraron resultados directos para: %1").arg(query));
    }

    return ToolResult::success(results.join('\n'));
}

// --- 2. HTTP FETCH ---
QJsonObject HttpFetchTool::parametersSchema() const {
    QJsonObject properties{
        {"url", QJsonObject{
            {"type", "string"},
            {"description", "URL completa (http:// o https://) cuyo contenido se desea leer."}
        }}
    };
    return QJsonObject{
        {"type", "object"},
        {"properties", properties},
        {"required", QJsonArray{"url"}}
    };
}

ToolResult HttpFetchTool::execute(const QJsonObject &arguments) {
    const QString urlStr = arguments.value("url").toString().trimmed();
    const QUrl url(urlStr);
    if (!url.isValid() || (url.scheme() != QStringLiteral("http") && url.scheme() != QStringLiteral("https"))) {
        return ToolResult::error(QStringLiteral("URL inválida: %1").arg(urlStr));
    }

    QString netErr;
    const QByteArray rawData = performSyncGet(url, 15000, netErr);
    if (rawData.isEmpty()) {
        return ToolResult::error(QStringLiteral("No se pudo descargar '%1': %2").arg(urlStr, netErr));
    }

    QString text = htmlToCleanText(QString::fromUtf8(rawData));
    constexpr int maxChars = 8000;
    if (text.length() > maxChars) {
        text = text.left(maxChars) + QStringLiteral("\n[... Contenido web truncado a 8000 caracteres ...]");
    }

    return ToolResult::success(text);
}

void registerWebTools(ToolRegistry &registry) {
    registry.registerTool(std::make_unique<WebSearchTool>());
    registry.registerTool(std::make_unique<HttpFetchTool>());
}
