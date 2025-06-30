#include "CameraView.h"

#include <QVideoWidget>
#include <QCamera>
#include <QMessageBox>

#if QT_CONFIG(permissions)
#include <QCoreApplication>
#include <QPermission>
static bool hasPermission = false;
#endif

ViewFinder::ViewFinder(QWidget* parent)
    : QWidget(parent) {
    ui_videoWidget = new QVideoWidget(this);
    setMinimumHeight(200);
}

QVideoSink* ViewFinder::videoSink() const {
    return ui_videoWidget->videoSink();
}

void ViewFinder::resizeEvent(QResizeEvent* event) {
    if (ui_videoWidget) {
        const QRect parentRect = rect();
        const int parentWidth = parentRect.width();
        const int parentHeight = parentRect.height();

        // 计算16:9的宽高
        int w = parentWidth;
        int h = parentWidth * 9 / 16;

        if (h > parentHeight) {
            h = parentHeight;
            w = parentHeight * 16 / 9;
        }

        // 居中显示
        const int x = (parentWidth - w) / 2;
        const int y = (parentHeight - h) / 2;

        ui_videoWidget->setGeometry(x, y, w, h);
    }
    QWidget::resizeEvent(event);
}

CameraView::CameraView(QWidget* parent)
    : BarCard(tr("Camera Viewer"), ":/res/icons/camera.svg", Bottom, parent) {
    initUI();
    m_captureSession = new QMediaCaptureSession(this);
    m_captureSession->setVideoOutput(ui_viewfinder);
}

void CameraView::initCamera(const QCameraDevice& device, const QCameraFormat& format) {
#if QT_CONFIG(permissions)
    QCameraPermission cameraPermission;
    switch (qApp->checkPermission(cameraPermission)) {
        case Qt::PermissionStatus::Undetermined:
            qApp->requestPermission(cameraPermission, [=] {
                initCamera(device, format);
            });
            return;
        case Qt::PermissionStatus::Denied:
            QMessageBox::warning(this, tr("Camera Permission Denied"),
                                 tr("Please allow camera access in the system settings."));
            return;
        case Qt::PermissionStatus::Granted:
            hasPermission = true;
            break;
    }
#endif
    updateCamera(device, format);
}

QString format2String(const QCameraFormat& cameraFormat) {
    return QString("%1×%2@%3fps, %4")
            .arg(cameraFormat.resolution().width())
            .arg(cameraFormat.resolution().height())
            .arg(cameraFormat.maxFrameRate())
            .arg(QVideoFrameFormat::pixelFormatToString(cameraFormat.pixelFormat()));
}

void CameraView::updateCamera(const QCameraDevice& device, const QCameraFormat& format) {
    m_camera.reset(new QCamera(device, this));
    m_camera->setCameraFormat(format);
    m_captureSession->setCamera(m_camera.data());

    connect(
        m_camera.data(), &QCamera::errorOccurred,
        this, [=](QCamera::Error error, const QString& errorString) {
            qDebug() << "Camera Error:" << error << errorString;
        }
    );

    if (m_running)
        m_camera->start();
}

void CameraView::updateFormat(const QCameraFormat& format) {
    if (!m_camera || (m_camera->cameraFormat() == format)) return;
    m_camera->setCameraFormat(format);
}

void CameraView::start() {
#if QT_CONFIG(permissions)
    if (!hasPermission) {
        QMessageBox::warning(this, tr("Camera Permission Denied"),
                             tr("Please allow camera access in the system settings."));
        return;
    }
#endif
    if (!m_camera) return;
    m_camera->start();
    m_running = true;
}

void CameraView::stop() {
    if (!m_camera) return;
    m_camera->stop();
    m_running = false;
}

void CameraView::initUI() {
    ui_viewfinder = new ViewFinder();
    auto* contentLayout = new QVBoxLayout(ui_content);
    contentLayout->addWidget(ui_viewfinder);
}
