#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <QFontDatabase>

class ThemeManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString currentPreset READ currentPreset WRITE applyPreset NOTIFY themeChanged)
    Q_PROPERTY(QStringList availablePresets READ availablePresets CONSTANT)

    // Paleta de colores reactiva
    Q_PROPERTY(QString bgDarkest READ bgDarkest WRITE setBgDarkest NOTIFY themeChanged)
    Q_PROPERTY(QString bgSidebar READ bgSidebar WRITE setBgSidebar NOTIFY themeChanged)
    Q_PROPERTY(QString bgBase READ bgBase WRITE setBgBase NOTIFY themeChanged)
    Q_PROPERTY(QString bgSurface READ bgSurface WRITE setBgSurface NOTIFY themeChanged)
    Q_PROPERTY(QString borderColor READ borderColor WRITE setBorderColor NOTIFY themeChanged)
    Q_PROPERTY(QString textPrimary READ textPrimary WRITE setTextPrimary NOTIFY themeChanged)
    Q_PROPERTY(QString textSecondary READ textSecondary WRITE setTextSecondary NOTIFY themeChanged)
    Q_PROPERTY(QString accentColor READ accentColor WRITE setAccentColor NOTIFY themeChanged)
    Q_PROPERTY(QString userBubbleColor READ userBubbleColor WRITE setUserBubbleColor NOTIFY themeChanged)

    // Tipografía
    Q_PROPERTY(QString fontFamily READ fontFamily WRITE setFontFamily NOTIFY themeChanged)
    Q_PROPERTY(QString monoFontFamily READ monoFontFamily WRITE setMonoFontFamily NOTIFY themeChanged)
    Q_PROPERTY(int baseFontSize READ baseFontSize WRITE setBaseFontSize NOTIFY themeChanged)
    Q_PROPERTY(QStringList systemFonts READ systemFonts CONSTANT)

public:
    explicit ThemeManager(QObject *parent = nullptr);

    QString currentPreset() const { return m_currentPreset; }
    QStringList availablePresets() const {
        return {
            QStringLiteral("Catppuccin Mocha"),
            QStringLiteral("Tokyo Night"),
            QStringLiteral("Rose Pine"),
            QStringLiteral("Gruvbox Dark"),
            QStringLiteral("Nord Polar"),
            QStringLiteral("Personalizado")
        };
    }

    QString bgDarkest() const { return m_bgDarkest; }
    QString bgSidebar() const { return m_bgSidebar; }
    QString bgBase() const { return m_bgBase; }
    QString bgSurface() const { return m_bgSurface; }
    QString borderColor() const { return m_borderColor; }
    QString textPrimary() const { return m_textPrimary; }
    QString textSecondary() const { return m_textSecondary; }
    QString accentColor() const { return m_accentColor; }
    QString userBubbleColor() const { return m_userBubbleColor; }

    QString fontFamily() const { return m_fontFamily; }
    QString monoFontFamily() const { return m_monoFontFamily; }
    int baseFontSize() const { return m_baseFontSize; }
    QStringList systemFonts() const { return QFontDatabase::families(); }

    Q_INVOKABLE void applyPreset(const QString &presetName);
    Q_INVOKABLE void resetToDefault();

    void setBgDarkest(const QString &c);
    void setBgSidebar(const QString &c);
    void setBgBase(const QString &c);
    void setBgSurface(const QString &c);
    void setBorderColor(const QString &c);
    void setTextPrimary(const QString &c);
    void setTextSecondary(const QString &c);
    void setAccentColor(const QString &c);
    void setUserBubbleColor(const QString &c);

    void setFontFamily(const QString &f);
    void setMonoFontFamily(const QString &f);
    void setBaseFontSize(int s);

signals:
    void themeChanged();

private:
    void loadSettings();
    void saveSettings();

    QString m_currentPreset = QStringLiteral("Catppuccin Mocha");
    QString m_bgDarkest = QStringLiteral("#11111b");
    QString m_bgSidebar = QStringLiteral("#181825");
    QString m_bgBase = QStringLiteral("#1e1e2e");
    QString m_bgSurface = QStringLiteral("#313244");
    QString m_borderColor = QStringLiteral("#45475a");
    QString m_textPrimary = QStringLiteral("#cdd6f4");
    QString m_textSecondary = QStringLiteral("#a6adc8");
    QString m_accentColor = QStringLiteral("#cba6f7");
    QString m_userBubbleColor = QStringLiteral("#89b4fa");

    QString m_fontFamily = QStringLiteral("Sans Serif");
    QString m_monoFontFamily = QStringLiteral("monospace");
    int m_baseFontSize = 14;
};
