#pragma once
#include <QObject>
#include <QString>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

class MarkdownParser : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(MarkdownParser)

    Q_PROPERTY(QString markdown READ markdown WRITE setMarkdown NOTIFY markdownChanged)
    Q_PROPERTY(QVariantList blocks READ blocks NOTIFY blocksChanged)

public:
    explicit MarkdownParser(QObject *parent = nullptr);

    QString markdown() const { return m_markdown; }
    void setMarkdown(const QString &md);

    QVariantList blocks() const { return m_blocks; }

    Q_INVOKABLE void copyToClipboard(const QString &text) const;

signals:
    void markdownChanged();
    void blocksChanged();

private:
    static QString highlightCode(const QString &code, const QString &language);
    static QString styleHtmlForQt(const QString &rawHtml);

    QString m_markdown;
    QVariantList m_blocks;
};
