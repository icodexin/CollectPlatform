#include "CoSettingsMgr.h"

void CoSettingsMgr::flush() {
    instance().SettingsManager::flush();
}

QString CoSettingsMgr::serverHostname() {
    return instance().getValueImpl("network/server/host", "localhost").toString();
}

QString CoSettingsMgr::eegAddress() {
    return instance().getValueImpl("device/eeg/address", "127.0.0.1").toString();
}

int CoSettingsMgr::eegPort() {
    return instance().getValueImpl("device/eeg/port", 8844).toInt();
}

int CoSettingsMgr::bandPort() {
    return instance().getValueImpl("device/band/port", 12345).toInt();
}

QString CoSettingsMgr::mqttAddress() {
    return instance().getValueImpl("network/mqtt/address", "127.0.0.1").toString();
}

int CoSettingsMgr::mqttPort() {
    return instance().getValueImpl("network/mqtt/port", 1883).toInt();
}

void CoSettingsMgr::setServerHostname(const QString& host) {
    return instance().setValueImpl("network/server/host", host);
}

void CoSettingsMgr::setEEGAddress(const QString& address) {
    return instance().setValueImpl("device/eeg/address", address);
}

void CoSettingsMgr::setEEGPort(const int port) {
    return instance().setValueImpl("device/eeg/port", port);
}

void CoSettingsMgr::setBandPort(const int port) {
    return instance().setValueImpl("device/band/port", port);
}

void CoSettingsMgr::setMQTTAddress(const QString& address) {
    return instance().setValueImpl("network/mqtt/address", address);
}

void CoSettingsMgr::setMQTTPort(const int port) {
    return instance().setValueImpl("network/mqtt/port", port);
}
