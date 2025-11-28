#include "MuSettingsMgr.h"

namespace {
    constexpr int kDefaultNavMenuCompactMode = 0; // HusMenu.CompactMode.Mode_Relaxed
    constexpr auto kDefaultAppDarkMode = HusTheme::DarkMode::Dark;
}

MuSettingsMgr* MuSettingsMgr::instance() {
    static auto* _instance = new MuSettingsMgr();
    return _instance;
}

MuSettingsMgr* MuSettingsMgr::create(QQmlEngine*, QJSEngine*) {
    return instance();
}

int MuSettingsMgr::navMenuCompactMode() const {
    return getValueImpl("ui/app/navMenuCompactMode", kDefaultNavMenuCompactMode).toInt();
}

HusTheme::DarkMode MuSettingsMgr::appDarkMode() const {
    return getValueImpl("ui/app/darkMode", QVariant::fromValue(kDefaultAppDarkMode)).value<HusTheme::DarkMode>();
}

void MuSettingsMgr::setNavMenuCompactMode(const int mode) {
    setValueImpl("ui/app/navMenuCompactMode", mode);
}

void MuSettingsMgr::setAppDarkMode(const HusTheme::DarkMode mode) {
    setValueImpl("ui/app/darkMode", QVariant::fromValue(mode));
}

MuSettingsMgr::MuSettingsMgr(QObject* parent) : QObject(parent) {
}
