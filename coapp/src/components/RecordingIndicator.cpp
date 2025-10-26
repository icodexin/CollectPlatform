#include "RecordingIndicator.h"
#include <QLabel>
#include <QTime>
#include <QTimer>
#include <QPainter>
#include <QBoxLayout>
#include <QPainterPath>

namespace {
    constexpr auto kInactiveColor = Qt::red;
    constexpr auto kWaitingColor = QColor(252, 149, 39);
    constexpr auto kRecordingColor = Qt::darkGreen;
    constexpr auto kRecordingColor1 = Qt::gray;

    QTime getElapsedTime(const QTime& start) {
        const int elapsed = start.msecsTo(QTime::currentTime());
        const int roundedSecs = (elapsed + 500) / 1000 * 1000;
        return QTime::fromMSecsSinceStartOfDay(roundedSecs);
    }
}

class RecordingIconWidget final : public QWidget {
public:
    explicit RecordingIconWidget(QWidget* parent = nullptr)
        : QWidget(parent) {
        const int size = fontMetrics().height();
        initUI({size, size});
    }

    RecordingIconWidget(const int w, const int h, QWidget* parent = nullptr)
        : RecordingIconWidget({w, h}, parent) {
    }

    explicit RecordingIconWidget(const QSize& size, QWidget* parent = nullptr)
        : QWidget(parent) {
        initUI(size);
    }

    explicit RecordingIconWidget(const int size, QWidget* parent = nullptr)
        : RecordingIconWidget(size, size, parent) {
    }

public slots:
    void triggerInterval() {
        if (m_recording) {
            if (m_color == kRecordingColor) {
                m_color = kRecordingColor1;
            } else {
                m_color = kRecordingColor;
            }
            update();
        }
    }

    void setStatus(const RecordingIndicator::Status status) {
        switch (status) {
            case RecordingIndicator::Inactive:
                m_recording = false;
                m_color = kInactiveColor;
                break;
            case RecordingIndicator::Starting:
                m_recording = true;
                m_color = kWaitingColor;
                break;
            case RecordingIndicator::Recording:
                m_recording = true;
                m_color = kRecordingColor;
                break;
            case RecordingIndicator::Stopping:
                m_recording = false;
                m_color = kWaitingColor;
                break;
        }
        update();
    }

protected:
    void paintEvent(QPaintEvent* event) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setBrush(m_color);
        p.setPen(Qt::NoPen);

        const int w = width();
        const int h = height();
        const int side = qMin(w, h);
        const qreal radius = side / 2.0;
        const QPointF center(w / 2.0, h / 2.0);

        QPainterPath ringPath;
        ringPath.addEllipse(center, radius, radius);
        ringPath.addEllipse(center, radius * 0.8, radius * 0.8);
        ringPath.setFillRule(Qt::OddEvenFill);
        p.drawPath(ringPath);

        if (m_recording) {
            p.drawEllipse(center, radius * 0.3, radius * 0.3);
        } else {
            p.translate(center);
            p.drawRect(-radius * 0.3, -radius * 0.3, radius * 0.6, radius * 0.6);
        }
    }

private:
    void initUI(const QSize& size) {
        setFixedSize(size);
        setStatus(RecordingIndicator::Inactive);
    }

private:
    bool m_recording = false;
    QColor m_color = kInactiveColor;
};

RecordingIndicator::RecordingIndicator(QWidget* parent, const bool recordTime)
    : QWidget(parent), m_recordTime(recordTime) {
    initUI();
    setStatus(Inactive);
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &RecordingIndicator::handleTimeout);
}

void RecordingIndicator::start(const bool immediate) {
    if (m_timer->isActive())
        m_timer->stop();
    setStatus(Starting);
    if (immediate)
        onStarted();
}

void RecordingIndicator::stop(const bool immediate) {
    if (m_timer->isActive())
        m_timer->stop();
    setStatus(Stopping);
    if (immediate)
        onStopped();
}

void RecordingIndicator::setHint(const QString& inactive, const QString& starting, const QString& recording,
    const QString& stopping) {
    setInactiveHint(inactive);
    setStartingHint(starting);
    setRecordingHint(recording);
    setStoppingHint(stopping);
}

void RecordingIndicator::setInactiveHint(const QString& hint) {
    m_inactiveHint = hint;
    if (m_status == Inactive)
        ui_hintLabel->setText(m_inactiveHint);
}

void RecordingIndicator::setStartingHint(const QString& hint) {
    m_startingHint = hint;
    if (m_status == Starting)
        ui_hintLabel->setText(m_startingHint);
}

void RecordingIndicator::setRecordingHint(const QString& hint) {
    m_recordingHint = hint;
    if (m_status == Recording)
        ui_hintLabel->setText(m_recordingHint);
}

void RecordingIndicator::setStoppingHint(const QString& hint) {
    m_stoppingHint = hint;
    if (m_status == Stopping)
        ui_hintLabel->setText(m_stoppingHint);
}

void RecordingIndicator::setRecordTime(const bool recordTime) {
    m_recordTime = recordTime;
    ui_timeLabel->setVisible(m_recordTime && m_status == Recording);
}

void RecordingIndicator::onStarted(const bool success) {
    if (success) {
        m_startTime = QTime::currentTime();
        m_timer->start(1000);
        setStatus(Recording);
    } else {
        setStatus(Inactive);
    }
}

void RecordingIndicator::onStopped() {
    setStatus(Inactive);
}

void RecordingIndicator::initUI() {
    const auto layout = new QHBoxLayout(this);
    layout->setSpacing(2);
    layout->setContentsMargins(0, 0, 0, 0);

    ui_iconWidget = new RecordingIconWidget(fontMetrics().height());
    ui_timeLabel = new QLabel();
    ui_timeLabel->setVisible(false);
    ui_timeLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    ui_hintLabel = new QLabel(m_inactiveHint);
    ui_hintLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    layout->addWidget(ui_iconWidget);
    layout->addWidget(ui_timeLabel);
    layout->addWidget(ui_hintLabel);
}

void RecordingIndicator::setStatus(const Status status) {
    if (status != m_status) {
        m_status = status;
        switch (status) {
            case Inactive:
                ui_timeLabel->setVisible(false);
                ui_hintLabel->setText(m_inactiveHint);
                break;
            case Starting:
                ui_timeLabel->setVisible(false);
                ui_timeLabel->setText("00:00:00");
                ui_hintLabel->setText(m_startingHint);
                break;
            case Recording:
                ui_timeLabel->setVisible(m_recordTime);
                ui_hintLabel->setText(m_recordingHint);
                break;
            case Stopping:
                ui_timeLabel->setVisible(false);
                ui_hintLabel->setText(m_stoppingHint);
                break;
        }
        ui_iconWidget->setStatus(status);
    }
}

void RecordingIndicator::handleTimeout() const {
    if (m_recordTime) {
        ui_timeLabel->setText(getElapsedTime(m_startTime).toString("hh:mm:ss"));
    }
    ui_iconWidget->triggerInterval();
}
