#ifndef CAMERASETTINGPANEL_H
#define CAMERASETTINGPANEL_H

#include <QtCore/QByteArray>
#include <QtCore/QSize>
#include <QtMultimedia/QCameraDevice>
#include <QtMultimedia/QMediaDevices>
#include <QtWidgets/QGroupBox>

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
    void loadPersistedSettings();
    bool isSavedFormat(const QCameraFormat& format) const;
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
    QByteArray m_savedDeviceId;
    QSize m_savedFormatRes;
    qreal m_savedFormatMaxFps = 0;
    QVideoFrameFormat::PixelFormat m_savedFormatPixelFormat = QVideoFrameFormat::Format_Invalid;
};
#endif // CAMERASETTINGPANEL_H
