#include "SettingsManager.h"

SettingsManager::SettingsManager() {
    qDebug() << "App settings will be saved at: " << m_settings.fileName();
}

SettingsManager::~SettingsManager() {
    flush();
}

void SettingsManager::flush() {
    QMutexLocker locker(&m_mutex);
    for (const QString& key : m_dirtyKeys) {
        m_settings.setValue(key, m_cache.value(key));
    }
    m_settings.sync();
    m_dirtyKeys.clear();
}

QVariant SettingsManager::getValueImpl(const QString& key, const QVariant& defaultValue) const {
    QMutexLocker locker(&m_mutex);

    if (m_cache.contains(key)) {
        return m_cache.value(key);
    }

    const QVariant value = m_settings.value(key, defaultValue);
    m_cache.insert(key, value);
    return value;
}

void SettingsManager::setValueImpl(const QString& key, const QVariant& value) {
    QMutexLocker locker(&m_mutex);
    m_cache.insert(key, value);
    m_dirtyKeys.insert(key);
}
