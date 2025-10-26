#include "SettingsManager.h"

SettingsManager& SettingsManager::instance() {
    static SettingsManager instance;
    return instance;
}

void SettingsManager::flush() {
    instance().flushImpl();
}

QString SettingsManager::serverHostname() {
    return instance().getValueImpl("network/server/host", "localhost").toString();
}

QString SettingsManager::eegAddress() {
    return instance().getValueImpl("device/eeg/address", "127.0.0.1").toString();
}

int SettingsManager::eegPort() {
    return instance().getValueImpl("device/eeg/port", 8844).toInt();
}

int SettingsManager::bandPort() {
    return instance().getValueImpl("device/band/port", 12345).toInt();
}

QString SettingsManager::mqttAddress() {
    return instance().getValueImpl("network/mqtt/address", "127.0.0.1").toString();
}

int SettingsManager::mqttPort() {
    return instance().getValueImpl("network/mqtt/port", 1883).toInt();
}

void SettingsManager::setServerHostname(const QString& host) {
    return instance().setValueImpl("network/server/host", host);
}

void SettingsManager::setEEGAddress(const QString& address) {
    return instance().setValueImpl("device/eeg/address", address);
}

void SettingsManager::setEEGPort(const int port) {
    return instance().setValueImpl("device/eeg/port", port);
}

void SettingsManager::setBandPort(const int port) {
    return instance().setValueImpl("device/band/port", port);
}

void SettingsManager::setMQTTAddress(const QString& address) {
    return instance().setValueImpl("network/mqtt/address", address);
}

void SettingsManager::setMQTTPort(const int port) {
    return instance().setValueImpl("network/mqtt/port", port);
}

QVariant SettingsManager::getValueImpl(const QString& key, const QVariant& defaultValue) const {
    QMutexLocker locker(&m_mutex);

    if (m_cache.contains(key))
        return m_cache.value(key);

    const QVariant var = m_settings.value(key, defaultValue);
    m_cache.insert(key, var);
    return var;
}

SettingsManager::SettingsManager() {
    qDebug() << "App settings saved in path:" << m_settings.fileName();
}

SettingsManager::~SettingsManager() {
    flushImpl();
}

void SettingsManager::setValueImpl(const QString& key, const QVariant& value) {
    QMutexLocker locker(&m_mutex);
    m_cache.insert(key, value);
    m_dirtyKeys.insert(key);
}

void SettingsManager::flushImpl() {
    QMutexLocker locker(&m_mutex);
    for (const QString& key : m_dirtyKeys)
        m_settings.setValue(key, m_cache.value(key));
    m_settings.sync();
    m_dirtyKeys.clear();
}
