#ifndef SETTINGVIEW_H
#define SETTINGVIEW_H

#include <QtMultimedia/QCameraDevice>
#include <QtWidgets/QWidget>
#include "services/VideoPushService.h"

class InfoPanel;
class EEGSettingPanel;
class BandSettingPanel;
class CameraSettingPanel;
class StreamSettingPanel;
class MqttSettingPanel;

class SettingView final : public QWidget {
    Q_OBJECT

public:
    explicit SettingView(QWidget* parent = nullptr);
    QString eegAddress() const;
    int eegPort() const;
    int bandServicePort() const;
    QCameraDevice cameraDevice() const;
    QCameraFormat cameraFormat() const;
    QString mqttUserId() const;

signals:
    void requestConnectEEG(const QString& address, int port);
    void requestDisconnectEEG();
    void requestStartBandService(int port);
    void requestStopBandService();
    void requestOpenCamera(const QCameraDevice& device, const QCameraFormat& format);
    void requestCloseCamera();
    void requestUpdateCameraDevice(const QCameraDevice& device);
    void requestUpdateCameraFormat(const QCameraFormat& format);
    void requestUpdateCamera(const QCameraDevice& device, const QCameraFormat& format);
    void requestStartMqtt(const QString& address, int port, const QString& id,
                          const QString& username, const QString& password);
    void requestStopMqtt();
    void requestStartVideoPush(const PushConfig& config);
    void requestStopVideoPush();

public slots:
    void onEEGConnected() const;
    void onEEGDisconnected() const;
    void onEEGReceiverRunningChanged(bool connected) const;
    void onBandServiceStarted() const;
    void onBandServiceStopped() const;
    void onBandServiceRunningChanged(bool running) const;
    void onCameraOpened() const;
    void onCameraClosed() const;
    void onCameraRunningChanged(bool running) const;
    void onMqttConnected() const;
    void onMqttDisconnected() const;
    void onVideoPushStateChanged(PushWorkerState state) const;

private:
    void initUI();
    void initConnection();

private:
    EEGSettingPanel* ui_eegPanel = nullptr;
    BandSettingPanel* ui_bandPanel = nullptr;
    CameraSettingPanel* ui_cameraPanel = nullptr;
    StreamSettingPanel* ui_streamPanel = nullptr;
    MqttSettingPanel* ui_mqttPanel = nullptr;
    InfoPanel* ui_infoPanel = nullptr;
};

#endif //SETTINGVIEW_H
