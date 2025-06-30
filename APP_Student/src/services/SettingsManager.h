#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QHostAddress>
#include <QString>
#include <QSettings>

class SettingsManager {
public:
    SettingsManager(const SettingsManager&) = delete;

    SettingsManager& operator=(const SettingsManager&) = delete;

    static SettingsManager& instance();

    QString serverHost() const;

    void setServerHost(const QHostAddress& host);

    QString eegHost() const;

    void setEegHost(const QHostAddress& host);

    int eegPort() const;

    void setEegPort(int port);

    QString bandHost() const;

    void setBandHost(const QHostAddress& host);

    int bandPort() const;

    void setBandPort(int port);

private:
    SettingsManager() = default;

    ~SettingsManager() = default;

    QSettings m_settings;
};

#endif //SETTINGSMANAGER_H
