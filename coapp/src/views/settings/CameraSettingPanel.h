#ifndef CAMERASETTINGPANEL_H
#define CAMERASETTINGPANEL_H

#include <QGroupBox>
#include <QCameraDevice>
#include <QMediaDevices>

class QComboBox;
class QPushButton;

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
#endif // CAMERASETTINGPANEL_H
