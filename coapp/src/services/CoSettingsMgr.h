#ifndef COSETTINGSMGR_H
#define COSETTINGSMGR_H

#include "SettingsManager.h"
#include "singleton.h"

class CoSettingsMgr final: public SettingsManager, public Singleton<CoSettingsMgr> {
    DECLARE_SINGLETON(CoSettingsMgr)

public:
    template<typename T>
    static T getValue(const QString& key, const T& defaultValue = T{}) {
        return instance().SettingsManager::getValue(key, defaultValue);
    }

    template<typename T>
    static void setValue(const QString& key, const T& value) {
        instance().SettingsManager::setValue(key, value);
    }

    static void flush();

    static QString serverHostname();
    static QString eegAddress();
    static int eegPort();
    static int bandPort();
    static QString mqttAddress();
    static int mqttPort();

    static void setServerHostname(const QString& host);
    static void setEEGAddress(const QString& address);
    static void setEEGPort(int port);
    static void setBandPort(int port);
    static void setMQTTAddress(const QString& address);
    static void setMQTTPort(int port);

private:
    CoSettingsMgr() = default;
    ~CoSettingsMgr() override = default;
};

#endif //COSETTINGSMGR_H
