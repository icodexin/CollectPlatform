#ifndef COSETTINGSMGR_H
#define COSETTINGSMGR_H

#include <QtMultimedia/QVideoFrameFormat>

#include "SettingsManager.h"
#include "singleton.h"

#include "services/FFmpegHelper.h"

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

    static QUrl httpBaseUrl();

    static QString serverHostname();
    static QString eegAddress();
    static int eegPort();
    static int bandPort();
    static QString mqttAddress();
    static int mqttPort();
    static QString authAccessToken();
    static QString authRefreshToken();
    static QString authTokenType();
    static QString authUnifiedId();
    static QByteArray cameraDeviceId();
    static QSize cameraFormatRes();
    static qreal cameraFormatMaxFps();
    static QVideoFrameFormat::PixelFormat cameraFormatPixelFormat();

    static void setServerHostname(const QString& host);
    static void setEEGAddress(const QString& address);
    static void setEEGPort(int port);
    static void setBandPort(int port);
    static void setMQTTAddress(const QString& address);
    static void setMQTTPort(int port);
    static void setAuthAccessToken(const QString& token);
    static void setAuthRefreshToken(const QString& token);
    static void setAuthTokenType(const QString& tokenType);
    static void setAuthUnifiedId(const QString& unifiedId);
    static void setCameraDeviceId(const QByteArray& deviceId);
    static void setCameraFormatRes(const QSize& res);
    static void setCameraFormatMaxFps(qreal fps);
    static void setCameraFormatPixelFormat(QVideoFrameFormat::PixelFormat pixelFormat);

    static QString streamUrl();
    static QSize streamRes();
    static int streamFps();
    static int streamBitrateKbps();
    static FFProfile streamProfile();
    static int streamGopSize();
    static int streamBFrames();
    static FFEncoderType streamEncoderType();

    static QString streamx264Preset();
    static QString streamx264Tune();

    static bool streamVtRealtime();
    static bool streamVtAllowFallback();
    static bool streamVtPrioritizeSpeed();
    static bool streamVtSpatialAQ();

    static QString streamQsvPreset();
    static int streamQsvAsyncDepth();

    static QString streamNvPreset();
    static QString streamNvTune();
    static int streamNvLookahead();

    static void setStreamUrl(const QString& url);
    static void setStreamRes(const QSize& res);
    static void setStreamFps(int fps);
    static void setStreamBitrateKbps(int kbps);
    static void setStreamProfile(FFProfile profile);
    static void setStreamGopSize(int gop);
    static void setStreamBFrames(int bframes);
    static void setStreamEncoderType(FFEncoderType type);

    static void setStreamx264Preset(const QString& preset);
    static void setStreamx264Tune(const QString& tune);

    static void setStreamVtRealtime(bool v);
    static void setStreamVtAllowFallback(bool v);
    static void setStreamVtPrioritizeSpeed(bool v);
    static void setStreamVtSpatialAQ(bool v);

    static void setStreamQsvPreset(const QString& preset);
    static void setStreamQsvAsyncDepth(int depth);

    static void setStreamNvPreset(const QString& preset);
    static void setStreamNvTune(const QString& tune);
    static void setStreamNvLookahead(int n);

private:
    CoSettingsMgr() = default;
    ~CoSettingsMgr() override = default;
};

#endif //COSETTINGSMGR_H
