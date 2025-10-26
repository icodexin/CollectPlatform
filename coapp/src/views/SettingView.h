#ifndef SETTINGVIEW_H
#define SETTINGVIEW_H

#include <QWidget>
#include <QCameraDevice>
#include <QGroupBox>
#include <QMediaDevices>

class SettingView;
class QPushButton;
class IPv4Edit;
class QLineEdit;
class QSpinBox;
class QComboBox;

class EEGSettingPanel final : public QGroupBox {
    Q_OBJECT

public:
    explicit EEGSettingPanel(QWidget* parent = nullptr);
    QString address() const;
    int port() const;

signals:
    void requestConnect(const QString& address, int port);
    void requestDisconnect();

public slots:
    void handleConnected() const;
    void handleDisconnected() const;

private slots:
    void onConnectBtnClicked();

private:
    IPv4Edit* m_addressEdit = nullptr;
    QSpinBox* m_portSpinBox = nullptr;
    QPushButton* m_connectBtn = nullptr;
};

class BandSettingPanel final : public QGroupBox {
    Q_OBJECT

public:
    explicit BandSettingPanel(QWidget* parent = nullptr);
    int port() const;

signals:
    void requestStartService(int port);
    void requestStopService();

public slots:
    void handleServiceStarted() const;
    void handleServiceStopped() const;
    void handleServiceRunningChanged(bool running) const;

private slots:
    void onListenBtnClicked();

private:
    QSpinBox* m_portSpinBox = nullptr;
    QPushButton* m_listenBtn = nullptr;
};

class CameraSettingPanel final : public QGroupBox {
    Q_OBJECT

public:
    explicit CameraSettingPanel(QWidget* parent = nullptr);
    QCameraDevice device() const;
    QCameraFormat format() const;

signals:
    void requestOpen(const QCameraDevice& device, const QCameraFormat& format);
    void requestClose();
    void requestUpdateDevice(const QCameraDevice& device);
    void requestUpdateFormat(const QCameraFormat& format);

public slots:
    void handleOpened() const;
    void handleClosed() const;
    void handleRunningChanged(bool running) const;

private slots:
    void onOpenBtnClicked();
    void updateCurrentDevice();
    void updateCurrentFormat();

private:
    void setDevice(const QCameraDevice& device);
    void setFormat(const QCameraFormat& format);
    void updateDeviceComboBox();
    void updateFormatComboBox();
    void updateOpenBtn() const;

private:
    QComboBox* m_deviceComboBox = nullptr;
    QComboBox* m_formatComboBox = nullptr;
    QPushButton* m_openBtn = nullptr;
    QMediaDevices m_mediaDevices;
    QCameraDevice m_device;
    QCameraFormat m_format;
};

class MqttSettingPanel final : public QGroupBox {
    Q_OBJECT

public:
    explicit MqttSettingPanel(QWidget* parent = nullptr);
    QString address() const;
    int port() const;
    QString id() const;
    QString username() const;
    QString password() const;

signals:
    void requestConnect(const QString& address, int port, const QString& id,
                        const QString& username, const QString& password);
    void requestDisconnect();

public slots:
    void handleConnected() const;
    void handleDisconnected() const;

private slots:
    void onConnectBtnClicked();

private:
    IPv4Edit* m_addressEdit = nullptr;
    QSpinBox* m_portSpinBox = nullptr;
    QLineEdit* m_idEdit = nullptr;
    QLineEdit* m_usernameEdit = nullptr;
    QLineEdit* m_passwordEdit = nullptr;
    QPushButton* m_connectBtn = nullptr;
};

class SettingView final : public QWidget {
    Q_OBJECT

public:
    explicit SettingView(QWidget* parent = nullptr);
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
    void requestStartMqtt(const QString& address, int port, const QString& id,
                         const QString& username, const QString& password);
    void requestStopMqtt();

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

private:
    void initUI();
    void initConnection();

private:
    EEGSettingPanel* ui_eegPanel = nullptr;
    BandSettingPanel* ui_bandPanel = nullptr;
    CameraSettingPanel* ui_cameraPanel = nullptr;
    MqttSettingPanel* ui_mqttPanel = nullptr;
};

#endif //SETTINGVIEW_H
