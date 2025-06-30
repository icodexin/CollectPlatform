#include "StatusIndicator.h"
#include <QHBoxLayout>
#include <QTimer>

StatusIndicator::StatusIndicator(const QString& runHint, const QString& haltHint,
                                 const bool recordTime, QWidget* parent)
    : QWidget(parent), m_runHint(runHint), m_haltHint(haltHint), m_needRecordTime(recordTime),
      m_timer(new QTimer(this)) {
    initUI();
    connect(m_timer, &QTimer::timeout, this, &StatusIndicator::onTimerTimeout);
}

void StatusIndicator::startRecording(const QString& hint_text) {
    m_counter = 0;
    m_time = QTime::currentTime();
    m_timer->start(1000);
    ui_iconLabel->setPixmap(m_recordingIcon);

    if (!hint_text.isEmpty()) {
        m_runHint = hint_text;
    }
    ui_hintLabel->setText(m_needRecordTime ? QString("00:00:00 %1").arg(m_runHint) : m_runHint);
    update();
}

void StatusIndicator::stopRecording() {
    if (m_timer->isActive())
        m_timer->stop();
    ui_iconLabel->setPixmap(m_unrecordingIcon);
    ui_hintLabel->setText(m_haltHint);
    update();
}

void StatusIndicator::setRunHint(const QString& text) {
    m_runHint = text;
}

void StatusIndicator::onTimerTimeout() {
    m_counter++;
    if (m_needRecordTime) {
        const int elapsed = m_time.msecsTo(QTime::currentTime());
        const QString elapsed_str = QTime::fromMSecsSinceStartOfDay(elapsed).toString("hh:mm:ss");
        ui_hintLabel->setText(QString("%1 %2").arg(elapsed_str, m_runHint));
    } else {
        ui_hintLabel->setText(m_runHint);
    }

    // 切换图标
    ui_iconLabel->setPixmap((m_counter % 2) ? m_lightRecordingIcon : m_recordingIcon);
    update();
}

void StatusIndicator::initUI() {
    auto layout = new QHBoxLayout(this);
    layout->setSpacing(2);
    layout->setContentsMargins(0, 0, 0, 0);

    const int icon_height = fontMetrics().height();

    m_recordingIcon = QIcon(":/res/icons/record-circle-outline.svg").pixmap(icon_height, icon_height);
    m_lightRecordingIcon = QIcon(":/res/icons/record-circle-outline-light.svg").pixmap(icon_height, icon_height);
    m_unrecordingIcon = QIcon(":/res/icons/stop-circle-outline.svg").pixmap(icon_height, icon_height);
    ui_iconLabel = new QLabel(this);
    ui_iconLabel->setPixmap(m_unrecordingIcon);
    ui_iconLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    ui_hintLabel = new QLabel(m_haltHint, this);
    ui_hintLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui_hintLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    layout->addWidget(ui_iconLabel);
    layout->addWidget(ui_hintLabel, 1);
}
