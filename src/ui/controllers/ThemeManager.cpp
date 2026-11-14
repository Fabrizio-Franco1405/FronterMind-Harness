#include "ThemeManager.h"
#include <QSettings>

ThemeManager::ThemeManager(QObject *parent) : QObject(parent) {
    loadSettings();
}

void ThemeManager::applyPreset(const QString &presetName) {
    m_currentPreset = presetName;

    if (presetName == QStringLiteral("Catppuccin Mocha")) {
        m_bgDarkest       = QStringLiteral("#11111b");
        m_bgSidebar       = QStringLiteral("#181825");
        m_bgBase          = QStringLiteral("#1e1e2e");
        m_bgSurface       = QStringLiteral("#313244");
        m_borderColor     = QStringLiteral("#45475a");
        m_textPrimary     = QStringLiteral("#cdd6f4");
        m_textSecondary   = QStringLiteral("#a6adc8");
        m_accentColor     = QStringLiteral("#cba6f7");
        m_userBubbleColor = QStringLiteral("#89b4fa");
    } else if (presetName == QStringLiteral("Tokyo Night")) {
        m_bgDarkest       = QStringLiteral("#13141c");
        m_bgSidebar       = QStringLiteral("#16161e");
        m_bgBase          = QStringLiteral("#1a1b26");
        m_bgSurface       = QStringLiteral("#24283b");
        m_borderColor     = QStringLiteral("#414868");
        m_textPrimary     = QStringLiteral("#c0caf5");
        m_textSecondary   = QStringLiteral("#9aa5ce");
        m_accentColor     = QStringLiteral("#7aa2f7");
        m_userBubbleColor = QStringLiteral("#bb9af7");
    } else if (presetName == QStringLiteral("Rose Pine")) {
        m_bgDarkest       = QStringLiteral("#12101a");
        m_bgSidebar       = QStringLiteral("#191724");
        m_bgBase          = QStringLiteral("#1f1d2e");
        m_bgSurface       = QStringLiteral("#26233a");
        m_borderColor     = QStringLiteral("#403d52");
        m_textPrimary     = QStringLiteral("#e0def4");
        m_textSecondary   = QStringLiteral("#908caa");
        m_accentColor     = QStringLiteral("#ebbcba");
        m_userBubbleColor = QStringLiteral("#c4a7e7");
    } else if (presetName == QStringLiteral("Gruvbox Dark")) {
        m_bgDarkest       = QStringLiteral("#141617");
        m_bgSidebar       = QStringLiteral("#1d2021");
        m_bgBase          = QStringLiteral("#282828");
        m_bgSurface       = QStringLiteral("#3c3836");
        m_borderColor     = QStringLiteral("#504945");
        m_textPrimary     = QStringLiteral("#ebdbb2");
        m_textSecondary   = QStringLiteral("#bdae93");
        m_accentColor     = QStringLiteral("#fe8019");
        m_userBubbleColor = QStringLiteral("#83a598");
    } else if (presetName == QStringLiteral("Nord Polar")) {
        m_bgDarkest       = QStringLiteral("#242933");
        m_bgSidebar       = QStringLiteral("#2e3440");
        m_bgBase          = QStringLiteral("#3b4252");
        m_bgSurface       = QStringLiteral("#434c5e");
        m_borderColor     = QStringLiteral("#4c566a");
        m_textPrimary     = QStringLiteral("#eceff4");
        m_textSecondary   = QStringLiteral("#d8dee9");
        m_accentColor     = QStringLiteral("#88c0d0");
        m_userBubbleColor = QStringLiteral("#81a1c1");
    }

    emit themeChanged();
    saveSettings();
}

void ThemeManager::resetToDefault() {
    m_fontFamily = QStringLiteral("Sans Serif");
    m_monoFontFamily = QStringLiteral("monospace");
    m_baseFontSize = 14;
    applyPreset(QStringLiteral("Catppuccin Mocha"));
}

#define DEFINE_COLOR_SETTER(FuncName, MemberVar) \
void ThemeManager::FuncName(const QString &c) { \
    if (MemberVar == c || c.trimmed().isEmpty()) return; \
    MemberVar = c.trimmed(); \
    m_currentPreset = QStringLiteral("Personalizado"); \
    emit themeChanged(); \
    saveSettings(); \
}

DEFINE_COLOR_SETTER(setBgDarkest, m_bgDarkest)
DEFINE_COLOR_SETTER(setBgSidebar, m_bgSidebar)
DEFINE_COLOR_SETTER(setBgBase, m_bgBase)
DEFINE_COLOR_SETTER(setBgSurface, m_bgSurface)
DEFINE_COLOR_SETTER(setBorderColor, m_borderColor)
DEFINE_COLOR_SETTER(setTextPrimary, m_textPrimary)
DEFINE_COLOR_SETTER(setTextSecondary, m_textSecondary)
DEFINE_COLOR_SETTER(setAccentColor, m_accentColor)
DEFINE_COLOR_SETTER(setUserBubbleColor, m_userBubbleColor)

void ThemeManager::setFontFamily(const QString &f) {
    if (m_fontFamily == f || f.isEmpty()) return;
    m_fontFamily = f;
    emit themeChanged();
    saveSettings();
}

void ThemeManager::setMonoFontFamily(const QString &f) {
    if (m_monoFontFamily == f || f.isEmpty()) return;
    m_monoFontFamily = f;
    emit themeChanged();
    saveSettings();
}

void ThemeManager::setBaseFontSize(int s) {
    s = qBound(11, s, 22);
    if (m_baseFontSize == s) return;
    m_baseFontSize = s;
    emit themeChanged();
    saveSettings();
}

void ThemeManager::saveSettings() {
    QSettings settings("FronterMind", "Harness");
    settings.beginGroup("Theme");
    settings.setValue("preset", m_currentPreset);
    settings.setValue("bgDarkest", m_bgDarkest);
    settings.setValue("bgSidebar", m_bgSidebar);
    settings.setValue("bgBase", m_bgBase);
    settings.setValue("bgSurface", m_bgSurface);
    settings.setValue("borderColor", m_borderColor);
    settings.setValue("textPrimary", m_textPrimary);
    settings.setValue("textSecondary", m_textSecondary);
    settings.setValue("accentColor", m_accentColor);
    settings.setValue("userBubbleColor", m_userBubbleColor);
    settings.setValue("fontFamily", m_fontFamily);
    settings.setValue("monoFontFamily", m_monoFontFamily);
    settings.setValue("baseFontSize", m_baseFontSize);
    settings.endGroup();
}

void ThemeManager::loadSettings() {
    QSettings settings("FronterMind", "Harness");
    settings.beginGroup("Theme");
    m_currentPreset   = settings.value("preset", "Catppuccin Mocha").toString();
    m_bgDarkest       = settings.value("bgDarkest", "#11111b").toString();
    m_bgSidebar       = settings.value("bgSidebar", "#181825").toString();
    m_bgBase          = settings.value("bgBase", "#1e1e2e").toString();
    m_bgSurface       = settings.value("bgSurface", "#313244").toString();
    m_borderColor     = settings.value("borderColor", "#45475a").toString();
    m_textPrimary     = settings.value("textPrimary", "#cdd6f4").toString();
    m_textSecondary   = settings.value("textSecondary", "#a6adc8").toString();
    m_accentColor     = settings.value("accentColor", "#cba6f7").toString();
    m_userBubbleColor = settings.value("userBubbleColor", "#89b4fa").toString();
    m_fontFamily      = settings.value("fontFamily", "Sans Serif").toString();
    m_monoFontFamily  = settings.value("monoFontFamily", "monospace").toString();
    m_baseFontSize    = settings.value("baseFontSize", 14).toInt();
    settings.endGroup();
}
