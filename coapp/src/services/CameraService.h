#ifndef CAMERASERVICE_H
#define CAMERASERVICE_H

#include <QObject>
#include <QCamera>

class QVideoSink;

class CameraService final: public QObject {
    Q_OBJECT

public:
    explicit CameraService(QObject* parent = nullptr);

signals:
    void runningChanged(bool);
    void errorOccurred(QCamera::Error error, const QString& errorString);
    void videoFrameChanged(const QVideoFrame& frame);

public slots:
    void start();
    void stop();
    void updateCamera(const QCameraDevice& device, const QCameraFormat& format);
    void updateFormat(const QCameraFormat& format);

private slots:
    void setRunning(bool);
    void onCameraActiveChanged(bool);
    void onCameraErrorOccurred(QCamera::Error, const QString&);

private:
    QMediaCaptureSession* m_captureSession;
    std::unique_ptr<QCamera> m_camera;
    QVideoSink* m_videoSink;
    bool m_running = false;
};


#endif //CAMERASERVICE_H