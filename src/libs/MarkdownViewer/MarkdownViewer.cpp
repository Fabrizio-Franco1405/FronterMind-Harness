// ============================================================================
// SECCIÓN 1: CABECERAS E INCLUSIONES DEL PARSER MARKDOWN (CMARK-GFM)
// ============================================================================
#include "MarkdownViewer.h"
#include <cmark-gfm.h>
#include <cmark-gfm-core-extensions.h>
#include <QGuiApplication>
#include <QClipboard>
#include <QRegularExpression>
#include <QSet>
#include <QVariantMap>

// ============================================================================
// SECCIÓN 2: REGISTRO DE EXTENSIONES GITHUB FLAVORED MARKDOWN (TABLAS, TAREAS)
// ============================================================================
MarkdownParser::MarkdownParser(QObject *parent) : QObject(parent) {
    static bool registered = false;
    if (!registered) {
        cmark_gfm_core_extensions_ensure_registered();
        registered = true;
    }
}

// ============================================================================
// SECCIÓN 3: UTILIDAD DE PORTAPAPELES PARA BOTONES "COPIAR CÓDIGO"
// ============================================================================
void MarkdownParser::copyToClipboard(const QString &text) const {
    if (QClipboard *clipboard = QGuiApplication::clipboard()) {
        clipboard->setText(text);
    }
}

// ============================================================================
// SECCIÓN 4: HOJA DE ESTILOS HTML INTERNA PARA EL MOTOR DE TEXTO DE QT6
// ============================================================================
QString MarkdownParser::styleHtmlForQt(const QString &rawHtml) {
    QString styled = rawHtml.trimmed();

    // --- 4.1: Párrafos con respiración cómoda ---
    styled.replace("<p>", "<p style=\"margin-top: 6px; margin-bottom: 8px; line-height: 145%; color: #cdd6f4;\">");

    // --- 4.2: Títulos con jerarquía visual Catppuccin (Lavanda / Azul / Malva) ---
    styled.replace("<h1>", "<h1 style=\"color: #cba6f7; font-size: 20px; font-weight: bold; margin-top: 14px; margin-bottom: 8px;\">");
    styled.replace("<h2>", "<h2 style=\"color: #b4befe; font-size: 17px; font-weight: bold; margin-top: 12px; margin-bottom: 6px;\">");
    styled.replace("<h3>", "<h3 style=\"color: #89b4fa; font-size: 15px; font-weight: bold; margin-top: 10px; margin-bottom: 4px;\">");
    styled.replace("<h4>", "<h4 style=\"color: #94e2d5; font-size: 14px; font-weight: bold; margin-top: 8px; margin-bottom: 4px;\">");

    // --- 4.3: Negritas y cursivas ---
    styled.replace("<strong>", "<strong style=\"color: #f5e0dc; font-weight: bold;\">");
    styled.replace("<em>", "<em style=\"color: #bac2de;\">");

    // --- 4.4: Listas con espaciado entre ítems ---
    styled.replace("<ul>", "<ul style=\"margin-top: 6px; margin-bottom: 8px; margin-left: 18px;\">");
    styled.replace("<ol>", "<ol style=\"margin-top: 6px; margin-bottom: 8px; margin-left: 18px;\">");
    styled.replace("<li>", "<li style=\"margin-bottom: 6px; line-height: 140%; color: #cdd6f4;\">");

    // --- 4.5: Código inline suave ---
    styled.replace("<code>",
        "<span style=\"background-color: #45475a; color: #f5c2e7; font-family: 'JetBrains Mono', 'Fira Code', monospace; font-size: 12px;\">&nbsp;");
    styled.replace("</code>", "&nbsp;</span>");

    // --- 4.6: Enlaces y citas (Blockquotes) ---
    styled.replace("<a href=", "<a style=\"color: #89b4fa; text-decoration: none; font-weight: bold;\" href=");
    styled.replace("<blockquote>",
        "<blockquote style=\"margin-top: 8px; margin-bottom: 8px; margin-left: 10px; padding-left: 10px; color: #a6adc8; font-style: italic;\">");

    // --- 4.7: Tablas GFM estilizadas ---
    styled.replace("<table>",
        "<table cellspacing=\"0\" cellpadding=\"8\" style=\"margin-top: 8px; margin-bottom: 10px; border: 1px solid #45475a; background-color: #1e1e2e;\">");
    styled.replace("<th>",
        "<th style=\"background-color: #181825; color: #cba6f7; font-weight: bold; border: 1px solid #45475a; padding: 6px 10px;\">");
    styled.replace("<td>",
        "<td style=\"color: #cdd6f4; border: 1px solid #313244; padding: 6px 10px;\">");

    // --- 4.8: Separadores horizontales ---
    styled.replace("<hr />", "<hr style=\"background-color: #45475a; height: 1px; border: none; margin-top: 12px; margin-bottom: 12px;\" />");

    return styled;
}

// ============================================================================
// SECCIÓN 5: RESALTADOR DE SINTAXIS PARA BLOQUES DE CÓDIGO FUENTE
// ============================================================================
QString MarkdownParser::highlightCode(const QString &code, const QString &language) {
    QString lang = language.toLower().trimmed();
    Q_UNUSED(lang);

    static const QSet<QString> keywords = {
        "alignas", "alignof", "and", "asm", "auto", "bool", "break", "case", "catch", "char",
        "class", "const", "constexpr", "const_cast", "continue", "decltype", "default", "delete",
        "do", "double", "dynamic_cast", "else", "enum", "explicit", "export", "extern", "false",
        "final", "float", "for", "friend", "goto", "if", "import", "inline", "int", "long",
        "mutable", "namespace", "new", "noexcept", "nullptr", "operator", "or", "override",
        "private", "protected", "public", "register", "reinterpret_cast", "return", "short",
        "signed", "sizeof", "static", "static_assert", "static_cast", "struct", "switch",
        "template", "this", "thread_local", "throw", "true", "try", "typedef", "typeid",
        "typename", "union", "unsigned", "using", "virtual", "void", "volatile", "wchar_t", "while",
        "def", "elif", "except", "finally", "from", "global", "in", "is", "lambda", "nonlocal",
        "not", "pass", "raise", "with", "yield", "async", "await", "None", "True", "False", "self",
        "function", "var", "let", "typeof", "instanceof", "property", "signal", "readonly", "id",
        "fn", "pub", "mut", "impl", "trait", "match", "package", "func", "interface", "defer",
        "sudo", "cd", "ls", "mkdir", "rm", "cp", "mv", "echo", "cat", "grep", "cmake", "ninja",
        "git", "g++", "gcc", "clang", "clang++", "python", "python3", "cargo", "npm", "pnpm"
    };

    static const QSet<QString> builtinTypes = {
        "std", "string", "vector", "map", "set", "unordered_map", "shared_ptr", "unique_ptr",
        "cout", "cin", "cerr", "endl", "size_t", "int32_t", "int64_t", "uint32_t", "uint64_t",
        "QObject", "QString", "QVariant", "QVariantList", "QVariantMap", "QList", "QMap",
        "QGuiApplication", "QQmlApplicationEngine", "Item", "Rectangle", "Text", "Column",
        "Row", "ColumnLayout", "RowLayout", "Repeater", "Loader", "ListView", "MouseArea",
        "Q_OBJECT", "Q_PROPERTY", "QML_NAMED_ELEMENT", "Q_INVOKABLE", "signals", "slots"
    };

    static const QRegularExpression tokenRegex(
        R"((//[^\n]*|/\*[\s\S]*?\*/|#\s*(?:include|pragma|define|ifdef|ifndef|endif|if|else|elif)\b[^\n]*|#[^\n]*|"(?:\\.|[^"\\])*"|'(?:\\.|[^'\\])*'|\b[A-Za-z_][A-Za-z0-9_]*(?=\s*\()|\b[A-Za-z_][A-Za-z0-9_]*\b|\b\d+(?:\.\d+)?(?:[fFlLuU]+)?\b|(?:^|\s)(?:--?[A-Za-z0-9_-]+)))",
        QRegularExpression::MultilineOption
    );

    QString result;
    result.reserve(code.size() * 2);
    int lastIdx = 0;

    auto it = tokenRegex.globalMatch(code);
    while (it.hasNext()) {
        auto match = it.next();
        if (match.capturedStart() > lastIdx) {
            result += code.mid(lastIdx, match.capturedStart() - lastIdx).toHtmlEscaped();
        }

        QString token = match.captured(0);
        QString escaped = token.toHtmlEscaped();

        int endPos = match.capturedEnd();
        bool isFunctionCall = (endPos < code.length() && code.at(endPos) == '(');

        if (token.startsWith("//") || token.startsWith("/*")) {
            result += QStringLiteral("<span style=\"color:#6c7086; font-style:italic;\">%1</span>").arg(escaped);
        } else if (token.startsWith("#")) {
            static const QRegularExpression cppPrep(R"(^(#\s*(?:include|pragma|define|ifdef|ifndef|endif|if|else|elif)\b)(.*)$)");
            auto prepMatch = cppPrep.match(token);
            if (prepMatch.hasMatch()) {
                QString directive = prepMatch.captured(1).toHtmlEscaped();
                QString rest = prepMatch.captured(2).toHtmlEscaped();
                result += QStringLiteral("<span style=\"color:#f5c2e7; font-weight:bold;\">%1</span><span style=\"color:#a6e3a1;\">%2</span>")
                              .arg(directive, rest);
            } else {
                result += QStringLiteral("<span style=\"color:#6c7086; font-style:italic;\">%1</span>").arg(escaped);
            }
        } else if (token.startsWith('"') || token.startsWith('\'')) {
            result += QStringLiteral("<span style=\"color:#a6e3a1;\">%1</span>").arg(escaped);
        } else if (token.trimmed().startsWith('-')) {
            result += QStringLiteral("<span style=\"color:#fab387;\">%1</span>").arg(escaped);
        } else if (token.at(0).isDigit()) {
            result += QStringLiteral("<span style=\"color:#fab387;\">%1</span>").arg(escaped);
        } else if (keywords.contains(token)) {
            result += QStringLiteral("<span style=\"color:#cba6f7; font-weight:bold;\">%1</span>").arg(escaped);
        } else if (builtinTypes.contains(token) || (token.at(0).isUpper() && token.length() > 1)) {
            result += QStringLiteral("<span style=\"color:#f9e2af;\">%1</span>").arg(escaped);
        } else if (isFunctionCall) {
            result += QStringLiteral("<span style=\"color:#89b4fa; font-weight:bold;\">%1</span>").arg(escaped);
        } else {
            result += escaped;
        }

        lastIdx = match.capturedEnd();
    }

    if (lastIdx < code.length()) {
        result += code.mid(lastIdx).toHtmlEscaped();
    }

    return QStringLiteral("<pre style=\"margin:0; line-height:135%; font-family:'JetBrains Mono', 'Fira Code', monospace; color:#cdd6f4;\">%1</pre>").arg(result);
}

// ============================================================================
// SECCIÓN 6: SANITIZACIÓN DE RESPUESTAS LLM (DESENVOLVER ```markdown EXTERNO)
// ============================================================================
static QString unwrapOuterMarkdownFence(QString raw) {
    QString trimmed = raw.trimmed();

    // 1. Eliminar bloques <think>...</think> cerrados o abiertos al inicio
    static const QRegularExpression thinkClosedRegex(R"(<think>[\s\S]*?</think>\s*)", QRegularExpression::CaseInsensitiveOption);
    trimmed.remove(thinkClosedRegex);
    trimmed = trimmed.trimmed();

    // 2. Si el LLM envolvió toda su respuesta en ```markdown ... ``` o ```md ... ```,
    // quitar la cerca exterior para que cmark-gfm renderice los encabezados y listas reales.
    static const QRegularExpression outerMdFenceRegex(
        R"(^```(?:markdown|md|text|txt)\s*\r?\n([\s\S]*?)(?:\r?\n```)?$)",
        QRegularExpression::CaseInsensitiveOption
    );
    auto match = outerMdFenceRegex.match(trimmed);
    if (match.hasMatch()) {
        trimmed = match.captured(1).trimmed();
    }

    return trimmed;
}

// ============================================================================
// SECCIÓN 7: PARSEO DEL AST DE MARKDOWN Y GENERACIÓN DE BLOQUES PARA QML
// ============================================================================
void MarkdownParser::setMarkdown(const QString &md) {
    if (m_markdown == md) return;
    m_markdown = md;
    emit markdownChanged();

    m_blocks.clear();

    // --- 7.1: Limpiar etiquetas <think> y cercas ```markdown envolventes ---
    QString cleanedMd = unwrapOuterMarkdownFence(md);

    // --- 7.2: Inicializar parser cmark-gfm con extensiones ---
    cmark_parser *parser = cmark_parser_new(CMARK_OPT_DEFAULT);
    cmark_parser_attach_syntax_extension(parser, cmark_find_syntax_extension("table"));
    cmark_parser_attach_syntax_extension(parser, cmark_find_syntax_extension("strikethrough"));
    cmark_parser_attach_syntax_extension(parser, cmark_find_syntax_extension("autolink"));
    cmark_parser_attach_syntax_extension(parser, cmark_find_syntax_extension("tasklist"));

    QByteArray utf8 = cleanedMd.toUtf8();
    cmark_parser_feed(parser, utf8.constData(), utf8.length());
    cmark_node *doc = cmark_parser_finish(parser);

    cmark_llist *extensions = cmark_parser_get_syntax_extensions(parser);

    // --- 7.3: Extraer bloques de código anidados dentro de listas o citas ---
    struct ExtractedCode {
        QString language;
        QString content;
    };
    QList<ExtractedCode> nestedCodes;

    cmark_iter *iter = cmark_iter_new(doc);
    cmark_event_type ev_type;
    QList<cmark_node*> nodesToReplace;

    while ((ev_type = cmark_iter_next(iter)) != CMARK_EVENT_DONE) {
        cmark_node *cur = cmark_iter_get_node(iter);
        if (ev_type == CMARK_EVENT_ENTER && cmark_node_get_type(cur) == CMARK_NODE_CODE_BLOCK) {
            if (cmark_node_parent(cur) != doc) {
                nodesToReplace.append(cur);
            }
        }
    }
    cmark_iter_free(iter);

    for (cmark_node *codeNode : nodesToReplace) {
        const char *lit = cmark_node_get_literal(codeNode);
        QString codeText = lit ? QString::fromUtf8(lit) : QString();
        while (codeText.endsWith('\n') || codeText.endsWith('\r')) codeText.chop(1);

        const char *lang = cmark_node_get_fence_info(codeNode);
        QString langStr = lang ? QString::fromUtf8(lang).trimmed().split(' ').first() : QString();

        int idx = nestedCodes.size();
        nestedCodes.append({langStr, codeText});

        cmark_node *placeholder = cmark_node_new(CMARK_NODE_PARAGRAPH);
        cmark_node *textNode = cmark_node_new(CMARK_NODE_TEXT);
        QByteArray marker = QStringLiteral("@@FRONTERMIND_CODE_%1@@").arg(idx).toUtf8();
        cmark_node_set_literal(textNode, marker.constData());
        cmark_node_append_child(placeholder, textNode);

        cmark_node_replace(codeNode, placeholder);
        cmark_node_free(codeNode);
    }

    // --- 7.4: Función auxiliar para volcar fragmentos de texto y código ---
    auto flushHtmlWithMarkers = [&](const QString &htmlChunk) {
        static const QRegularExpression markerRegex(R"((?:<p[^>]*>)?\s*@@FRONTERMIND_CODE_(\d+)@@\s*(?:</p>)?)");
        int lastPos = 0;
        auto matchIt = markerRegex.globalMatch(htmlChunk);

        while (matchIt.hasNext()) {
            auto m = matchIt.next();
            QString before = htmlChunk.mid(lastPos, m.capturedStart() - lastPos).trimmed();
            if (!before.isEmpty()) {
                QVariantMap textBlock;
                textBlock["type"] = "text";
                textBlock["content"] = styleHtmlForQt(before);
                m_blocks.append(textBlock);
            }

            int codeIdx = m.captured(1).toInt();
            if (codeIdx >= 0 && codeIdx < nestedCodes.size()) {
                const auto &ec = nestedCodes[codeIdx];
                if (!ec.content.trimmed().isEmpty()) {
                    QVariantMap codeBlock;
                    codeBlock["type"] = "code";
                    codeBlock["language"] = ec.language;
                    codeBlock["content"] = ec.content;
                    codeBlock["highlighted"] = highlightCode(ec.content, ec.language);
                    m_blocks.append(codeBlock);
                }
            }
            lastPos = m.capturedEnd();
        }

        QString remaining = htmlChunk.mid(lastPos).trimmed();
        if (!remaining.isEmpty()) {
            QVariantMap textBlock;
            textBlock["type"] = "text";
            textBlock["content"] = styleHtmlForQt(remaining);
            m_blocks.append(textBlock);
        }
    };

    // --- 7.5: Recorrido de nodos principales del documento ---
    cmark_node *child = cmark_node_first_child(doc);
    QString currentHtml;

    while (child) {
        cmark_node_type type = cmark_node_get_type(child);

        if (type == CMARK_NODE_CODE_BLOCK) {
            const char *lit = cmark_node_get_literal(child);
            QString codeText = lit ? QString::fromUtf8(lit) : QString();
            while (codeText.endsWith('\n') || codeText.endsWith('\r')) codeText.chop(1);

            const char *lang = cmark_node_get_fence_info(child);
            QString langStr = lang ? QString::fromUtf8(lang).trimmed().split(' ').first() : QString();

            // Si un nodo de código es de tipo "markdown" o "md", renderizar su interior como Markdown real
            if (langStr.compare("markdown", Qt::CaseInsensitive) == 0 ||
                langStr.compare("md", Qt::CaseInsensitive) == 0) {
                QByteArray innerUtf8 = codeText.toUtf8();
                cmark_node *innerDoc = cmark_parse_document(innerUtf8.constData(), innerUtf8.length(), CMARK_OPT_DEFAULT);
                char *innerHtml = cmark_render_html(innerDoc, CMARK_OPT_DEFAULT, extensions);
                if (innerHtml) {
                    currentHtml += QString::fromUtf8(innerHtml);
                    free(innerHtml);
                }
                cmark_node_free(innerDoc);
            } else if (!codeText.trimmed().isEmpty()) {
                if (!currentHtml.trimmed().isEmpty()) {
                    flushHtmlWithMarkers(currentHtml);
                    currentHtml.clear();
                }

                QVariantMap codeBlock;
                codeBlock["type"] = "code";
                codeBlock["language"] = langStr;
                codeBlock["content"] = codeText;
                codeBlock["highlighted"] = highlightCode(codeText, langStr);
                m_blocks.append(codeBlock);
            }
        } else {
            char *html_cstr = cmark_render_html(child, CMARK_OPT_DEFAULT, extensions);
            if (html_cstr) {
                currentHtml += QString::fromUtf8(html_cstr);
                free(html_cstr);
            }
        }
        child = cmark_node_next(child);
    }

    if (!currentHtml.trimmed().isEmpty()) {
        flushHtmlWithMarkers(currentHtml);
    }

    cmark_node_free(doc);
    cmark_parser_free(parser);

    emit blocksChanged();
}
