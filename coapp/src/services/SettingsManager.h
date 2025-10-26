#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QMutex>
#include <QSettings>

class SettingsManager {
public:
    SettingsManager(const SettingsManager& other) = delete;
    SettingsManager& operator=(const SettingsManager& other) = delete;

    static SettingsManager& instance();

    template<typename T>
    static T getValue(const QString& key, const T& defaultValue = T{}) {
        const QVariant var = instance().getValueImpl(key, QVariant::fromValue(defaultValue));
        if (var.canConvert<T>()) {
            return var.value<T>();
        }
        return defaultValue;
    }

    template<typename T>
    static void setValue(const QString& key, const T& value) {
        instance().setValueImpl(key, QVariant::fromValue(value));
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
    SettingsManager();
    ~SettingsManager();

    QVariant getValueImpl(const QString& key, const QVariant& defaultValue) const;
    void setValueImpl(const QString& key, const QVariant& value);
    void flushImpl();

private:
    QSettings m_settings;
    mutable QMutex m_mutex;
    mutable QVariantMap m_cache;
    QSet<QString> m_dirtyKeys;
};

#endif //SETTINGSMANAGER_H
