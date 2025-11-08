#include "CameraView.h"
#include <QBoxLayout>
#include <QLabel>
#include <QPainter>
#include "SettingView.h"

class ViewFinder final: public QWidget {
public:
    explicit ViewFinder(QWidget* parent = nullptr) : QWidget(parent) {
        initUI();
    }

    void setPlaying(const bool playing) {
        m_playing = playing;
        ui_hint->setVisible(!playing);
        update();
    }

    void setCurrentFrame(const QVideoFrame& frame) {
        m_currentFrame = frame;
        update();
    }

protected:
    void paintEvent(QPaintEvent* event) override {
        if (!m_playing || !m_currentFrame.isValid())
            return;

        // 计算16:9的宽高
        qreal w = width();
        qreal h = w * 9.0 / 16;
        if (h > height()) {
            h = height();
            w = h * 16.0 / 9;
        }
        // 居中显示
        qreal x = (width() - w) / 2.0;
        qreal y = (height() - h) / 2.0;

        if (w > 0 && h > 0) {
            QPainter painter(this);
            m_currentFrame.paint(&painter, {x, y, w, h}, m_framePaintOptions);
        }
    }

private:
    void initUI() {
        ui_hint = new QWidget(this);
        auto* hintLayout = new QVBoxLayout(ui_hint);
        auto* iconLabel = new QLabel;
        iconLabel->setPixmap(QIcon(":/res/icons/camera-off.svg").pixmap(32, 32));
        auto* textLabel = new QLabel(tr("Preview has been closed."));
        hintLayout->addWidget(iconLabel, 0, Qt::AlignHCenter);
        hintLayout->addWidget(textLabel, 0, Qt::AlignHCenter);

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addStretch(1);
        layout->addWidget(ui_hint);
        layout->addStretch(1);
    }

private:
    QWidget* ui_hint = nullptr;
    QVideoFrame::PaintOptions m_framePaintOptions{};
    QVideoFrame m_currentFrame{};
    bool m_playing = false;
};

CameraView::CameraView(QWidget* parent)
    : DeviceView(tr("Camera"), ":/res/icons/camera.svg", Bottom, parent) {
    initUI();
    initConnection();
}

void CameraView::setPlaying(const bool playing) {
    if (playing != m_playing) {
        m_playing = playing;
        emit playingChanged(playing);
    }
}

void CameraView::setFrame(const QVideoFrame& frame) {
    ui_viewFinder->setCurrentFrame(frame);
}

void CameraView::initUI() {
    setIndicatorHint(tr("Inactive"), {}, tr("Running"), {});
    ui_viewFinder = new ViewFinder;
    auto* contentLayout = new QVBoxLayout;
    contentLayout->addWidget(ui_viewFinder);
    setContentLayout(contentLayout);
}

void CameraView::initConnection() {
    connect(this, &CameraView::playingChanged, ui_viewFinder, &ViewFinder::setPlaying);
    connect(this, &CameraView::playingChanged, this, &CameraView::onConnectionStatusChanged);
}
