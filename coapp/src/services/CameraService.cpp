#include "CameraService.h"
#include <QMediaCaptureSession>
#include <QVideoSink>

#if QT_CONFIG(permissions)
#include <QCoreApplication>
#include <QPermissions>
#include <QMessageBox>
#include <memory>
#endif

namespace {
#if QT_CONFIG(permissions)
    enum CameraPermission {
        Granted, Denied, GrantedByUser, DeniedByUser // ByUser means the user made a choice during this app session.
    };

    void doAfterPermissionCheck(const CameraPermission permission, const std::function<void(bool)>& callback) {
        switch (permission) {
            case GrantedByUser:
#ifdef Q_OS_MACOS
                QMessageBox::information(nullptr, QObject::tr("Camera Permission"),
                    QObject::tr(
                        "Camera permission has been granted. You need to restart the application to take effect."));
#endif
            case Granted:
                callback(true);
                break;
            case Denied:
            case DeniedByUser:
                QMessageBox::warning(nullptr, QObject::tr("Camera Permission"),
                    QObject::tr("Camera permission has been denied. Please allow camera access in system settings."));
                callback(false);
                break;
        }
    }

    void checkCameraPermission(const std::function<void(bool)>& callback) {
        QCameraPermission cameraPermission;
        const auto status = qApp->checkPermission(cameraPermission);
        if (status == Qt::PermissionStatus::Granted) {
            doAfterPermissionCheck(Granted, callback);
        } else if (status == Qt::PermissionStatus::Denied) {
            doAfterPermissionCheck(Denied, callback);
        } else { // Undetermined
            qApp->requestPermission(cameraPermission, [callback](const QPermission& permission) {
                if (permission.status() == Qt::PermissionStatus::Granted) {
                    doAfterPermissionCheck(GrantedByUser, callback);
                } else {
                    doAfterPermissionCheck(DeniedByUser, callback);
                }
            });
        }
    }
#else
    void checkCameraPermission(const std::function<void(bool)>& callback) {
        callback(true);
    }
#endif
}


CameraService::CameraService(QObject* parent) : QObject(parent) {
    m_captureSession = new QMediaCaptureSession(this);
    m_videoSink = new QVideoSink(this);
    m_captureSession->setVideoSink(m_videoSink);

    connect(m_videoSink, &QVideoSink::videoFrameChanged, this, &CameraService::videoFrameChanged);
}

void CameraService::start() {
    if (!m_camera || !m_camera->isAvailable())
        return;

    checkCameraPermission([&](const bool granted) {
        if (granted) {
            m_camera->start();
            setRunning(true);
        }
    });
}

void CameraService::stop() {
    if (!m_camera || !m_camera->isAvailable())
        return;
    if (m_camera->isActive()) {
        m_camera->stop();
    }
    setRunning(false);
}

void CameraService::updateCamera(const QCameraDevice& device, const QCameraFormat& format) {
    m_camera = std::make_unique<QCamera>(device);
    m_camera->setCameraFormat(format);
    m_captureSession->setCamera(m_camera.get());

    connect(m_camera.get(), &QCamera::errorOccurred, this, &CameraService::onCameraErrorOccurred);
    connect(m_camera.get(), &QCamera::activeChanged, this, &CameraService::onCameraActiveChanged);

    if (m_running)
        m_camera->start();
}

void CameraService::updateFormat(const QCameraFormat& format) {
    if (!m_camera || (m_camera->cameraFormat() == format))
        return;
    m_camera->setCameraFormat(format);
}

void CameraService::setRunning(const bool running) {
    if (running != m_running) {
        m_running = running;
        emit runningChanged(running);
    }
}

void CameraService::onCameraActiveChanged(const bool active) {
    if (!active)
        setRunning(false);
}

void CameraService::onCameraErrorOccurred(const QCamera::Error error, const QString& errorString) {
    if (error != QCamera::NoError) {
        setRunning(false);
        emit errorOccurred(error, errorString);
    }
}
