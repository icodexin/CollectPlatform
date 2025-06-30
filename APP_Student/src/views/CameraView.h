#ifndef CAMERAVIEW_H
#define CAMERAVIEW_H

#include "components/BarCard.h"
#include <QMediaCaptureSession>
#include <QCamera>

class QCameraFormat;

class ViewFinder final : public QWidget {
    Q_OBJECT

public:
    explicit ViewFinder(QWidget* parent = nullptr);
    Q_INVOKABLE QVideoSink* videoSink() const;

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    class QVideoWidget* ui_videoWidget;
};


class CameraView final : public BarCard {
    Q_OBJECT

public:
    explicit CameraView(QWidget* parent = nullptr);

public slots:
    void initCamera(const QCameraDevice& device, const QCameraFormat& format);
    void updateCamera(const QCameraDevice& device, const QCameraFormat& format);
    void updateFormat(const QCameraFormat& format);

    void start();
    void stop();

private:
    ViewFinder* ui_viewfinder;
    QMediaCaptureSession* m_captureSession;
    QScopedPointer<QCamera> m_camera;

    bool m_running = false;

    void initUI();
};


#endif //CAMERAVIEW_H
