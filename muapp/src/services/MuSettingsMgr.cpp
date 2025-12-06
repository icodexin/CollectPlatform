#include "MuSettingsMgr.h"
#include <QtQml/QJSEngine>

namespace {
    constexpr int kDefaultNavMenuCompactMode = 0; // HusMenu.CompactMode.Mode_Relaxed
    constexpr auto kDefaultAppDarkMode = HusTheme::DarkMode::Light;
}

MuSettingsMgr* MuSettingsMgr::instance() {
    static MuSettingsMgr instance;
    return &instance;
}

MuSettingsMgr* MuSettingsMgr::create(QQmlEngine*, QJSEngine*) {
    auto* instance = MuSettingsMgr::instance();
    QJSEngine::setObjectOwnership(instance, QJSEngine::CppOwnership);
    return instance;
}

int MuSettingsMgr::navMenuCompactMode() const {
    return getValueImpl("ui/app/navMenuCompactMode", kDefaultNavMenuCompactMode).toInt();
}

HusTheme::DarkMode MuSettingsMgr::appDarkMode() const {
    return getValueImpl("ui/app/darkMode", QVariant::fromValue(kDefaultAppDarkMode)).value<HusTheme::DarkMode>();
}

void MuSettingsMgr::setNavMenuCompactMode(const int mode) {
    if (mode == navMenuCompactMode())
        return;
    setValueImpl("ui/app/navMenuCompactMode", mode);
    emit navMenuCompactModeChanged(mode);
}

void MuSettingsMgr::setAppDarkMode(const HusTheme::DarkMode mode) {
    if (mode == appDarkMode())
        return;
    setValueImpl("ui/app/darkMode", QVariant::fromValue(mode));
    emit appDarkModeChanged(mode);
}

MuSettingsMgr::MuSettingsMgr(QObject* parent) : QObject(parent) {
}
