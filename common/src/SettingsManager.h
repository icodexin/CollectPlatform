#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QMutex>
#include <QSettings>

class SettingsManager {
public:
    SettingsManager();
    virtual ~SettingsManager();

    template <typename T>
    T getValue(const QString& key, const T& defaultValue = T{}) {
        const QVariant var = getValueImpl(key, QVariant::fromValue(defaultValue));
        if (var.canConvert<T>()) {
            return var.value<T>();
        }
        return defaultValue;
    }

    template <typename T>
    void setValue(const QString& key, const T& value) {
        setValueImpl(key, QVariant::fromValue(value));
    }

    void flush();

protected:
    QVariant getValueImpl(const QString& key, const QVariant& defaultValue) const;
    void setValueImpl(const QString& key, const QVariant& value);

private:
    QSettings m_settings;
    mutable QMutex m_mutex;
    mutable QVariantHash m_cache;
    QSet<QString> m_dirtyKeys;
};

#endif //SETTINGSMANAGER_H
