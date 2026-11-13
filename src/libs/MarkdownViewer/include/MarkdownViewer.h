#pragma once
#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

class MarkdownParser : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(MarkdownParser)

    Q_PROPERTY(QString markdown READ markdown WRITE setMarkdown NOTIFY markdownChanged)
    Q_PROPERTY(QString html READ html NOTIFY htmlChanged)

public:
    explicit MarkdownParser(QObject *parent = nullptr);

    QString markdown() const { return m_markdown; }
    void setMarkdown(const QString &md);

    QString html() const { return m_html; }

signals:
    void markdownChanged();
    void htmlChanged();

private:
    QString m_markdown;
    QString m_html;
};
