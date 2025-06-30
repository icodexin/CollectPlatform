//
// Created by Lenovo on 25-6-17.
//

#include "SettingsManager.h"

SettingsManager& SettingsManager::instance() {
    static SettingsManager instance;
    return instance;
}

QString SettingsManager::serverHost() const {
    return m_settings.value("network/server/host", "8.148.69.192").toString();
}

void SettingsManager::setServerHost(const QHostAddress& host) {
    m_settings.setValue("network/server/host", host.toString());
}

QString SettingsManager::eegHost() const {
    return m_settings.value("device/eeg/host", "127.0.0.1").toString();
}

void SettingsManager::setEegHost(const QHostAddress& host) {
    m_settings.setValue("device/eeg/host", host.toString());
}

int SettingsManager::eegPort() const {
    return m_settings.value("device/eeg/port", 8844).toInt();
}

void SettingsManager::setEegPort(const int port) {
    m_settings.setValue("device/eeg/port", port);
}

QString SettingsManager::bandHost() const {
    return m_settings.value("device/band/host", "127.0.0.1").toString();
}

void SettingsManager::setBandHost(const QHostAddress& host) {
    m_settings.setValue("device/band/host", host.toString());
}

int SettingsManager::bandPort() const {
    return m_settings.value("device/band/port", 12345).toInt();
}

void SettingsManager::setBandPort(const int port) {
    m_settings.setValue("device/band/port", port);
}
