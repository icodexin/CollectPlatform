#include "RppgViewController.h"

#include <algorithm>

#include <QtCore/QJsonArray>
#include <QtCore/QDebug>
#include <QtCore/QVariant>
#include <QtCore/QThread>

namespace {
    constexpr int k_defaultSampleRate = 34; // rPPG 默认采样率
    constexpr int k_renderRate = 30;        // 30 FPS
}

RppgViewWorker::RppgViewWorker(QObject* parent) : QObject(parent) {
}

void RppgViewWorker::reset() {
    m_queue.clear();
}

void RppgViewWorker::pushData(const QJsonObject& data) {
    Q_ASSERT(thread() == QThread::currentThread());

    const QJsonArray heartWave = data.value("heart_waveform").toArray();
    const QJsonArray respWave = data.value("resp_waveform").toArray();

    if (heartWave.isEmpty() || respWave.isEmpty()) {
        m_queue.clear();
        qDebug() << "rPPG waveform is empty, skip this packet.";
        return;
    }

    const qsizetype frameCount = std::min(heartWave.size(), respWave.size());
    const int sampleRate = data.value("sampling_rate").toInt(k_defaultSampleRate);
    const qint64 startTimestamp = data.value("start_timestamp").toVariant().toLongLong();

    const double heartRate = data.value("heart_rate").toDouble(-1.0);
    const double respiratoryRate = data.value("respiratory_rate").toDouble(-1.0);

    if (sampleRate <= 0) {
        qWarning() << "Invalid rPPG sampling_rate:" << sampleRate;
        return;
    }

    const double ratio = static_cast<double>(sampleRate) / k_renderRate;
    double counter = 0.0;

    for (qsizetype i = 0; i < frameCount; ++i) {
        counter += 1.0;
        if (counter >= ratio) {
            const double heart = heartWave.at(i).toDouble();
            const double resp = respWave.at(i).toDouble();
            const qint64 timestamp = startTimestamp + static_cast<qint64>(1000.0 * i / sampleRate);

            m_queue.enqueue(RppgViewFrame(
                timestamp,
                heart,
                resp,
                heartRate,
                respiratoryRate
            ));
            counter -= ratio;
        }
    }
}

void RppgViewWorker::fetchNextFrame() {
    Q_ASSERT(thread() == QThread::currentThread());

    if (!m_queue.isEmpty()) {
        const RppgViewFrame frame = m_queue.dequeue();
        emit frameReady(frame);
    }
}

RppgViewController::RppgViewController(QObject* parent) : QObject(parent) {
    qRegisterMetaType<RppgViewFrame>("RppgViewFrame");
    connect(&m_timer, &QTimer::timeout, this, &RppgViewController::onTimeout);
}

RppgViewController::~RppgViewController() {
    stopRendering();
}

void RppgViewController::classBegin() {
}

void RppgViewController::componentComplete() {
    startRendering();
}

void RppgViewController::pushData(const QJsonObject& data, const QString& studentId) {
    Q_UNUSED(studentId)

    if (m_worker) {
        QMetaObject::invokeMethod(
            m_worker,
            &RppgViewWorker::pushData,
            Qt::QueuedConnection,
            data
        );
    } else {
        qWarning() << "Worker is not running, rPPG data is discarded.";
    }
}

void RppgViewController::startRendering() {
    if (m_worker || m_thread) {
        qWarning() << "Already in rendering state.";
        return;
    }

    m_worker = new RppgViewWorker();
    m_thread = new QThread(this);
    m_worker->moveToThread(m_thread);

    connect(m_worker, &RppgViewWorker::frameReady, this, &RppgViewController::onFrameReady);
    connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QObject::deleteLater);

    if (m_timer.isActive())
        m_timer.stop();

    m_thread->start();
    m_timer.start(1000 / k_renderRate);
}

void RppgViewController::stopRendering() {
    m_timer.stop();

    if (m_worker && m_thread) {
        m_thread->quit();
        m_thread->wait();
        m_worker = nullptr;
        m_thread = nullptr;
    } else {
        qWarning() << "Not in rendering state.";
    }
}

void RppgViewController::onTimeout() {
    if (m_worker) {
        QMetaObject::invokeMethod(
            m_worker,
            &RppgViewWorker::fetchNextFrame,
            Qt::QueuedConnection
        );
    } else {
        qWarning() << "Worker is not running, cannot fetch rPPG frame.";
    }
}

void RppgViewController::reset() {
    if (m_timer.isActive())
        m_timer.stop();

    if (m_worker) {
        QMetaObject::invokeMethod(
            m_worker,
            &RppgViewWorker::reset,
            Qt::BlockingQueuedConnection
        );
    }

    emit resetRequested();

    m_timer.start(1000 / k_renderRate);
}

void RppgViewController::onFrameReady(const RppgViewFrame& frame)
{
    emit frameUpdated(frame.toVariantMap());
}