#ifndef NAVIGATIONVIEW_H
#define NAVIGATIONVIEW_H

#include <QtMultimedia/QCameraDevice>
#include <QtWidgets/QWidget>
#include "services/VideoPushService.h"

class InfoPanel;
class EEGSettingPanel;
class BandSettingPanel;
class CameraSettingPanel;
class StreamSettingPanel;
class QFrame;
class QLabel;
class QPushButton;
class QResizeEvent;

class NavigationView final : public QWidget {
    Q_OBJECT

public:
    explicit NavigationView(QWidget* parent = nullptr);
    QString eegAddress() const;
    int eegPort() const;
    int bandServicePort() const;
    QCameraDevice cameraDevice() const;
    QCameraFormat cameraFormat() const;

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
    void requestReconnectMqtt();
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
    void onMqttConnected();
    void onMqttDisconnected();
    void onMqttError(const QString& message);
    void onVideoPushStateChanged(PushWorkerState state) const;

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void initUI();
    void initConnection();
    void updateMqttStatusCard();
    void updateMqttStatusCardLayout();

private:
    EEGSettingPanel* ui_eegPanel = nullptr;
    BandSettingPanel* ui_bandPanel = nullptr;
    CameraSettingPanel* ui_cameraPanel = nullptr;
    StreamSettingPanel* ui_streamPanel = nullptr;
    QFrame* ui_mqttStatusCard = nullptr;
    QLabel* ui_mqttStatusTitleLabel = nullptr;
    QLabel* ui_mqttStatusBodyLabel = nullptr;
    QPushButton* ui_mqttReconnectButton = nullptr;
    InfoPanel* ui_infoPanel = nullptr;
    bool m_mqttConnected = false;
    bool m_mqttShowStatusCard = false;
};

#endif //NAVIGATIONVIEW_H
