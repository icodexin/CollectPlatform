#ifndef SETTINGVIEW_H
#define SETTINGVIEW_H

#include <QWidget>
#include <QCameraDevice>
#include <QMediaDevices>

class SettingView;
class QPushButton;
class IPv4Edit;
class QLineEdit;
class QSpinBox;
class QComboBox;


class CameraSettingManager final : public QObject {
    Q_OBJECT

public:
    explicit CameraSettingManager(SettingView* parent);

    QCameraDevice device() const;
    QCameraFormat format() const;

signals:
    void deviceChanged(QCameraDevice device);
    void formatChanged(QCameraFormat format);

private slots:
    void updateCurrentDevice();
    void updateCurrentFormat();

private:
    QComboBox* ui_deviceComboBox;
    QComboBox* ui_formatComboBox;
    QPushButton* ui_openBtn;

    QMediaDevices m_mediaDevices;
    QCameraDevice m_cameraDevice;
    QCameraFormat m_cameraFormat;

    void setDevice(const QCameraDevice& device);
    void setFormat(const QCameraFormat& format);

    void updateDeviceComboBox();
    void updateFormatComboBox();
    void updateOpenBtn();
};

class SettingView final : public QWidget {
    Q_OBJECT

public:
    explicit SettingView(QWidget* parent = nullptr);

    QCameraDevice cameraDevice() const;
    QCameraFormat cameraFormat() const;

signals:
    void requestConnectEEG(const QString& address, int port);
    void requestDisconnectEEG();
    void requestStartBandService(int port);
    void requestStopBandService();
    void requestOpenCamera();
    void requestCloseCamera();

    void cameraDeviceChanged(const QCameraDevice& device);
    void cameraFormatChanged(const QCameraFormat& format);

public slots:
    void onEEGConnected() const;
    void onEEGDisconnected() const;
    void onBandServiceStarted() const;
    void onBandServiceStopped() const;

private slots:
    void onEEGConnectBtnClicked();
    void onBandListenBtnClicked();
    void onCameraOpenBtnClicked();

private:
    friend class CameraSettingManager;

    IPv4Edit* ui_eegAddressEdit;
    QSpinBox* ui_eegPortSpinBox;
    QPushButton* ui_eegConnectBtn;

    QSpinBox* ui_bandPortSpinBox;
    QPushButton* ui_bandListenBtn;

    QComboBox* ui_cameraDeviceComboBox;
    QComboBox* ui_cameraFormatComboBox;
    QPushButton* ui_cameraOpenBtn;
    CameraSettingManager* m_cameraManager;

    void initUI();
    void initConnection();
};


#endif //SETTINGVIEW_H
