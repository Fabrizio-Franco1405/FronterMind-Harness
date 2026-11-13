#include "MarkdownViewer.h"
#include <cmark-gfm.h>
#include <cmark-gfm-core-extensions.h>

MarkdownParser::MarkdownParser(QObject *parent) : QObject(parent) {
    static bool registered = false;
    if (!registered) {
        cmark_gfm_core_extensions_ensure_registered();
        registered = true;
    }
}

void MarkdownParser::setMarkdown(const QString &md) {
    if (m_markdown == md) return;
    m_markdown = md;
    emit markdownChanged();

    // 1. Crear el parser
    cmark_parser *parser = cmark_parser_new(CMARK_OPT_DEFAULT);

    // 2. Activar extensiones (tablas, tachado, autolinks, listas de tareas)
    cmark_parser_attach_syntax_extension(parser, cmark_find_syntax_extension("table"));
    cmark_parser_attach_syntax_extension(parser, cmark_find_syntax_extension("strikethrough"));
    cmark_parser_attach_syntax_extension(parser, cmark_find_syntax_extension("autolink"));
    cmark_parser_attach_syntax_extension(parser, cmark_find_syntax_extension("tasklist"));

    // 3. Parsear el texto
    QByteArray utf8 = md.toUtf8();
    cmark_parser_feed(parser, utf8.constData(), utf8.length());
    cmark_node *doc = cmark_parser_finish(parser);

    // 4. Renderizar a HTML
    char *html_cstr = cmark_render_html(doc, CMARK_OPT_DEFAULT, cmark_parser_get_syntax_extensions(parser));
    QString rawHtml = QString::fromUtf8(html_cstr);

    // 5. Limpiar memoria de C
    free(html_cstr);
    cmark_node_free(doc);
    cmark_parser_free(parser);

    // 6. Envolver en HTML para Qt
    m_html = "<html><body>" + rawHtml + "</body></html>";

    emit htmlChanged();
}
