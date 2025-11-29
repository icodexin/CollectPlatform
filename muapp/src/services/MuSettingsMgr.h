#ifndef MUSETTINGSMGR_H
#define MUSETTINGSMGR_H

#include <QtQml/qqml.h>
#include "SettingsManager.h"
#include "HuskarUI/theme/hustheme.h"

class MuSettingsMgr final : public QObject, public SettingsManager {
    Q_OBJECT
    QML_SINGLETON
    QML_ELEMENT
    Q_DISABLE_COPY_MOVE(MuSettingsMgr)
    Q_PROPERTY(int navMenuCompactMode READ navMenuCompactMode WRITE setNavMenuCompactMode)
    Q_PROPERTY(HusTheme::DarkMode appDarkMode READ appDarkMode WRITE setAppDarkMode)

public:
    static MuSettingsMgr* instance();
    static MuSettingsMgr* create(QQmlEngine*, QJSEngine*);

    Q_INVOKABLE int navMenuCompactMode() const;
    Q_INVOKABLE HusTheme::DarkMode appDarkMode() const;

    Q_INVOKABLE void setNavMenuCompactMode(int mode);
    Q_INVOKABLE void setAppDarkMode(HusTheme::DarkMode mode);

private:
    explicit MuSettingsMgr(QObject* parent = nullptr);
    ~MuSettingsMgr() override = default;
};

#endif //MUSETTINGSMGR_H
