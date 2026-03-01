#include "CoSettingsMgr.h"

#include <QtCore/QSize>

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

QByteArray CoSettingsMgr::cameraDeviceId() {
    return instance().getValueImpl("device/camera/device_id", QByteArray{}).toByteArray();
}

QSize CoSettingsMgr::cameraFormatRes() {
    return instance().getValueImpl("device/camera/format/res", QSize{}).toSize();
}

qreal CoSettingsMgr::cameraFormatMaxFps() {
    return instance().getValueImpl("device/camera/format/max_fps", 0.0).toDouble();
}

QVideoFrameFormat::PixelFormat CoSettingsMgr::cameraFormatPixelFormat() {
    return instance().getValueImpl("device/camera/format/pixel_format",
        QVideoFrameFormat::Format_Invalid).value<QVideoFrameFormat::PixelFormat>();
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

void CoSettingsMgr::setCameraDeviceId(const QByteArray& deviceId) {
    instance().setValueImpl("device/camera/device_id", deviceId);
}

void CoSettingsMgr::setCameraFormatRes(const QSize& res) {
    instance().setValueImpl("device/camera/format/res", res);
}

void CoSettingsMgr::setCameraFormatMaxFps(const qreal fps) {
    instance().setValueImpl("device/camera/format/max_fps", fps);
}

void CoSettingsMgr::setCameraFormatPixelFormat(const QVideoFrameFormat::PixelFormat pixelFormat) {
    instance().setValueImpl("device/camera/format/pixel_format", pixelFormat);
}

QString CoSettingsMgr::streamUrl() {
    return instance().getValueImpl("stream/url", "rtsp://127.0.0.1:8554/live").toString();
}

QSize CoSettingsMgr::streamRes() {
    return instance().getValueImpl("stream/res", QSize{1280, 720}).toSize();
}

int CoSettingsMgr::streamFps() {
    return instance().getValueImpl("stream/fps", 30).toInt();
}

int CoSettingsMgr::streamBitrateKbps() {
    return instance().getValueImpl("stream/bitrate_kbps", 2000).toInt();
}

FFProfile CoSettingsMgr::streamProfile() {
    return instance().getValueImpl("stream/profile", FF_PROFILE_BASELINE).value<FFProfile>();
}

int CoSettingsMgr::streamGopSize() {
    return instance().getValueImpl("stream/gop_size", 2).toInt();
}

int CoSettingsMgr::streamBFrames() {
    return instance().getValueImpl("stream/bframes", 0).toInt();
}

FFEncoderType CoSettingsMgr::streamEncoderType() {
    return instance().getValueImpl("stream/encoder_type", 0).value<FFEncoderType>();
}

QString CoSettingsMgr::streamx264Preset() {
    return instance().getValueImpl("stream/x264/preset", "ultrafast").toString();
}

QString CoSettingsMgr::streamx264Tune() {
    return instance().getValueImpl("stream/x264/tune", "zerolatency").toString();
}

bool CoSettingsMgr::streamVtRealtime() {
    return instance().getValueImpl("stream/vt/realtime", true).toBool();
}

bool CoSettingsMgr::streamVtAllowFallback() {
    return instance().getValueImpl("stream/vt/allow_fallback", false).toBool();
}

bool CoSettingsMgr::streamVtPrioritizeSpeed() {
    return instance().getValueImpl("stream/vt/prioritize_speed", true).toBool();
}

bool CoSettingsMgr::streamVtSpatialAQ() {
    return instance().getValueImpl("stream/vt/spatial_aq", false).toBool();
}

QString CoSettingsMgr::streamQsvPreset() {
    return instance().getValueImpl("stream/qsv/preset", "faster").toString();
}

int CoSettingsMgr::streamQsvAsyncDepth() {
    return instance().getValueImpl("stream/qsv/async_depth", 1).toInt();
}

QString CoSettingsMgr::streamNvPreset() {
    return instance().getValueImpl("stream/nv/preset", "p1").toString();
}

QString CoSettingsMgr::streamNvTune() {
    return instance().getValueImpl("stream/nv/tune", "ull").toString();
}

int CoSettingsMgr::streamNvLookahead() {
    return instance().getValueImpl("stream/nv/lookahead", 0).toInt();
}

void CoSettingsMgr::setStreamUrl(const QString& url) {
    instance().setValueImpl("stream/url", url);
}

void CoSettingsMgr::setStreamRes(const QSize& res) {
    instance().setValueImpl("stream/res", res);
}

void CoSettingsMgr::setStreamFps(const int fps) {
    instance().setValueImpl("stream/fps", fps);
}

void CoSettingsMgr::setStreamBitrateKbps(const int kbps) {
    instance().setValueImpl("stream/bitrate_kbps", kbps);
}

void CoSettingsMgr::setStreamProfile(const FFProfile profile) {
    instance().setValueImpl("stream/profile", profile);
}

void CoSettingsMgr::setStreamGopSize(const int gop) {
    instance().setValueImpl("stream/gop_size", gop);
}

void CoSettingsMgr::setStreamBFrames(int bframes) {
    instance().setValueImpl("stream/bframes", bframes);
}

void CoSettingsMgr::setStreamEncoderType(const FFEncoderType type) {
    instance().setValueImpl("stream/encoder_type", type);
}

void CoSettingsMgr::setStreamx264Preset(const QString& preset) {
    instance().setValueImpl("stream/x264/preset", preset);
}

void CoSettingsMgr::setStreamx264Tune(const QString& tune) {
    instance().setValueImpl("stream/x264/tune", tune);
}

void CoSettingsMgr::setStreamVtRealtime(const bool v) {
    instance().setValueImpl("stream/vt/realtime", v);
}

void CoSettingsMgr::setStreamVtAllowFallback(const bool v) {
    instance().setValueImpl("stream/vt/allow_fallback", v);
}

void CoSettingsMgr::setStreamVtPrioritizeSpeed(const bool v) {
    instance().setValueImpl("stream/vt/prioritize_speed", v);
}

void CoSettingsMgr::setStreamVtSpatialAQ(const bool v) {
    instance().setValueImpl("stream/vt/spatial_aq", v);
}

void CoSettingsMgr::setStreamQsvPreset(const QString& preset) {
    instance().setValueImpl("stream/qsv/preset", preset);
}

void CoSettingsMgr::setStreamQsvAsyncDepth(const int depth) {
    instance().setValueImpl("stream/qsv/async_depth", depth);
}

void CoSettingsMgr::setStreamNvPreset(const QString& preset) {
    instance().setValueImpl("stream/nv/preset", preset);
}

void CoSettingsMgr::setStreamNvTune(const QString& tune) {
    instance().setValueImpl("stream/nv/tune", tune);
}

void CoSettingsMgr::setStreamNvLookahead(const int n) {
    instance().setValueImpl("stream/nv/lookahead", n);
}
